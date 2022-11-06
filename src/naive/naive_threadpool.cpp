/*
 * naive_threadpool.cpp
 *
 * Copyright (c) 2011 - 2022 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../trace_utils.h"
#include "../thread_utils.h"

#include "naive_threadpool.h"
#include "naive_operation_queue_manager.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

#define XDISPATCH_TP_TRACE(pool, idle, active)                                 \
    XDISPATCH_TRACE() << (pool) << "[idle=" << (idle) << "|total=" << (active) \
                      << "] "
#define XDISPATCH_TP_WARNING(pool, idle, active)                               \
    XDISPATCH_WARNING() << (pool) << "[idle=" << (idle)                        \
                        << "|total=" << (active) << "] "

static thread_local ithreadpool* s_current_pool = nullptr;

ithreadpool*
ithreadpool::current()
{
    return s_current_pool;
}

void
ithreadpool::run_with_threadpool(operation& op, ithreadpool* pool)
{
    struct scoped_setter
    {
        scoped_setter(ithreadpool* pool)
          : m_previous(s_current_pool)
        {
            s_current_pool = pool;
        }

        ~scoped_setter() { s_current_pool = m_previous; }

    private:
        ithreadpool* m_previous;
    };

    scoped_setter pool_scope(pool);
    execute_operation_on_this_thread(op);
}

char const* const s_bucket_labels[threadpool::bucket_count] = {
    k_label_global_INTERACTIVE,
    k_label_global_INITIATED,
    k_label_global_UTILITY,
    k_label_global_BACKGROUND
};

class threadpool::data
{
public:
    data(threadpool* owner)
      : m_pool(owner)
      , m_operations_counter(0)
      , m_max_threads(0)
      , m_active_threads(0)
      , m_idle_threads(0)
      , m_operations()
      , m_cancelled(false)
    {
        XDISPATCH_ASSERT(m_max_threads.is_lock_free());
        XDISPATCH_ASSERT(m_active_threads.is_lock_free());
        XDISPATCH_ASSERT(m_idle_threads.is_lock_free());
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    threadpool* const m_pool;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    semaphore m_operations_counter;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<int> m_max_threads;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<int> m_active_threads;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<int> m_idle_threads;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::array<concurrentqueue<operation_ptr>, bucket_count> m_operations;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<bool> m_cancelled;
};

class threadpool::worker
{
public:
    explicit worker(const threadpool::data_ptr& data)
      : m_data(data)
      , m_thread(&worker::run, this)
    {}

    ~worker()
    {
        XDISPATCH_ASSERT(m_thread.joinable());
        m_thread.join();
    }

    std::thread::id get_id() const { return m_thread.get_id(); }

    void run()
    {
        static constexpr int skMaxSpinsBeforeSleep = 1000;
        static constexpr auto skMaxSleepBeforeThreadExit =
          std::chrono::seconds(30);

        int last_label = -1;
        while (!m_data->m_cancelled) {
            operation_ptr op;
            int label = -1;
            {
                // if no op we are idling and need to block on our op counter
                // NOLINTNEXTLINE(bugprone-branch-clone)
                if (m_data->m_operations_counter.try_acquire()) {
                    // all good go pick the operation
                } else if (m_data->m_operations_counter.spin_acquire(
                             skMaxSpinsBeforeSleep)) {
                    // all good go pick the operation
                } else {
                    if (trace_utils::is_debug_enabled()) {
                        thread_utils::set_current_thread_name("");
                        last_label = -1;
                    }

                    // FIXME(zwicker): We mark a thread as idle pretty late
                    // as it is technically idle during try_acquire() and
                    // spin_acquire() as well but this is kept in here for now
                    // to keep the fast path as simple as possible.
                    const auto idle_threads = m_data->m_idle_threads.fetch_add(
                                                1, std::memory_order_acq_rel) +
                                              1;
                    const auto active_threads =
                      m_data->m_active_threads.load(std::memory_order_consume);
                    XDISPATCH_TP_TRACE(
                      m_data->m_pool, idle_threads, active_threads)
                      << "Thread " << std::this_thread::get_id() << " idling";

                    // wait up to a timeout for the counter to acquire, if the
                    // timeout is reached we end this thread again to free
                    // resources in the system
                    if (m_data->m_operations_counter.wait_acquire(
                          skMaxSleepBeforeThreadExit)) {
                        // all good go pick the operation
                        m_data->m_idle_threads.fetch_sub(
                          1, std::memory_order_release);
                    } else {
                        // end this thread it seems there is no work remaining
                        m_data->m_idle_threads.fetch_sub(
                          1, std::memory_order_release);
                        m_data->m_active_threads.fetch_sub(
                          1, std::memory_order_release);

                        // Opportunistic recovery:
                        // There is a chance that while we made the decision to
                        // end (as no work seems remaining) such work was
                        // actually added while we updated the two counters
                        // above. In that case we must not end, because the
                        // entity adding work may have decided there is no need
                        // to trigger a new thread (as it still deemed us
                        // active). To recover we need to check a last time for
                        // pending work added since. Worst case this may cause
                        // one excess thread to be created which would then
                        // expire again eventually but not cause real harm.
                        if (m_data->m_operations_counter.try_acquire()) {
                            // found another operation, restore the active
                            // counter and go pick/execute that operation
                            m_data->m_active_threads.fetch_add(
                              1, std::memory_order_release);
                        } else {
                            break;
                        }
                    }
                }

                // search for the next operation starting with the highest
                // priority
                // Note: there has to be such operation as we acquired the
                //       semaphore above so if popping fails spontaneously
                //       we are good to repeat
                // FIXME(zwicker): This is unfair; if enough high prio ops get
                //                 queued we will never drain the lower prio ops
                for (label = 0; !m_data->m_cancelled; ++label) {
                    if (bucket_count == label) {
                        label = 0;
                    }

                    auto& ops_prio = m_data->m_operations[label];
                    ops_prio.try_dequeue(op);
                    if (op) {
                        break;
                    }
                }
                XDISPATCH_ASSERT(op);
            }

            if (op) {
                if (trace_utils::is_debug_enabled() && last_label != label) {
                    thread_utils::set_current_thread_name(
                      s_bucket_labels[label]);
                    last_label = label;
                }

                run_with_threadpool(*op, m_data->m_pool);
                op.reset();
            }
        }

        const auto remaining =
          m_data->m_active_threads.load(std::memory_order_consume);
        const auto idle =
          m_data->m_idle_threads.load(std::memory_order_consume);
        XDISPATCH_TP_TRACE(m_data->m_pool, remaining, idle)
          << "Thread" << std::this_thread::get_id() << " joining";
        operation_queue_manager::instance().detach(this);
    }

private:
    threadpool::data_ptr m_data;
    std::thread m_thread;
};

threadpool::threadpool()
  : ithreadpool()
  , m_data(std::make_shared<data>(this))
{
    // we are overcommitting by default so that it becomes less likely
    // that operations get starved due to threads blocking on resources
    m_data->m_max_threads =
      static_cast<int>(2 * thread_utils::system_thread_count());
    XDISPATCH_TRACE() << "threadpool with " << m_data->m_max_threads
                      << " system threads";
}

threadpool::~threadpool()
{
    m_data->m_cancelled = true;
    m_data->m_operations_counter.release(m_data->m_active_threads);
}

void
threadpool::execute(const operation_ptr& work, const queue_priority priority)
{
    int index = -1;
    switch (priority) {
        case queue_priority::USER_INTERACTIVE:
            index = bucket_USER_INTERACTIVE;
            break;
        case queue_priority::USER_INITIATED:
            index = bucket_USER_INITIATED;
            break;
        case queue_priority::UTILITY:
        case queue_priority::DEFAULT:
            index = bucket_UTILITY;
            break;
        case queue_priority::BACKGROUND:
            index = bucket_BACKGROUND;
            break;
    }

    XDISPATCH_ASSERT(index >= 0);
    const auto enqueued = m_data->m_operations[index].enqueue(work);
    XDISPATCH_ASSERT(enqueued);
    if (enqueued) {
        m_data->m_operations_counter.release();
    }
    schedule();
}

ithreadpool_ptr
backend::create_threadpool()
{
    return std::make_shared<threadpool>();
}

ithreadpool_ptr
backend::global_threadpool()
{
    // this is an intentional leak so that the destructor is ok to run from
    // within a pool thread
    static auto* s_instance = new ithreadpool_ptr(create_threadpool());
    return *s_instance;
}

void
threadpool::schedule()
{
    // lets check if there is an idle thread first
    const int active_threads =
      m_data->m_active_threads.load(std::memory_order_consume);
    const int idle_threads =
      m_data->m_idle_threads.load(std::memory_order_consume);
    XDISPATCH_ASSERT(idle_threads >= 0);
    XDISPATCH_ASSERT(active_threads >= 0);
    XDISPATCH_ASSERT(idle_threads <= active_threads &&
                     "We must never have more idle than active threads");

    if (idle_threads > 0) {
        // idle count will be decremented by thread waking up again
        XDISPATCH_TP_TRACE(this, active_threads, idle_threads)
          << "Woke an idle thread";
    }
    // check if we are good to create another thread
    else if (active_threads <
             m_data->m_max_threads.load(std::memory_order_consume)) {
        auto thread = std::make_shared<worker>(m_data);
        operation_queue_manager::instance().attach(thread);
        m_data->m_active_threads.fetch_add(1, std::memory_order_release);

        XDISPATCH_TP_TRACE(this, active_threads + 1, idle_threads)
          << "Spawned thread " << thread->get_id()
          << " (max=" << m_data->m_max_threads << ")";
    }
    // all threads busy and processor allocation reached, wait
    // and the operation will be picked up as soon as a thread is available
}

void
threadpool::notify_thread_blocked()
{
    const auto max_threads =
      m_data->m_max_threads.fetch_add(1, std::memory_order_acq_rel) + 1;
    const auto active =
      m_data->m_active_threads.load(std::memory_order_consume);
    const auto idle = m_data->m_idle_threads.load(std::memory_order_consume);
    XDISPATCH_TP_TRACE(this, active, idle)
      << "Increased threadcount to " << max_threads;
    schedule();
}

void
threadpool::notify_thread_unblocked()
{
    const auto max_threads =
      m_data->m_max_threads.fetch_sub(1, std::memory_order_acquire) - 1;
    ;
    const auto active =
      m_data->m_active_threads.load(std::memory_order_consume);
    const auto idle = m_data->m_idle_threads.load(std::memory_order_consume);
    XDISPATCH_TP_TRACE(this, active, idle)
      << "Lowered threadcount to " << max_threads;
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
