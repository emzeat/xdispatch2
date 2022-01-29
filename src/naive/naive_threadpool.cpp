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

#include "naive_threadpool.h"
#include "naive_operation_queue_manager.h"

#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

char const* const s_bucket_labels[threadpool::bucket_count] = {
    k_label_global_INTERACTIVE,
    k_label_global_INITIATED,
    k_label_global_UTILITY,
    k_label_global_BACKGROUND
};

class threadpool::data
{
public:
    data()
      : m_operations_counter(0)
      , m_max_threads(0)
      , m_active_threads(0)
      , m_idle_threads(0)
      , m_operations()
      , m_cancelled(false)
    {}

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

                    const auto idle_threads = ++m_data->m_idle_threads;
                    XDISPATCH_TRACE()
                      << std::this_thread::get_id() << " idling ("
                      << idle_threads << " idle)";

                    // wait up to a timeout for the counter to acquire, if the
                    // timeout is reached we end this thread again to free
                    // resources in the system
                    if (m_data->m_operations_counter.wait_acquire(
                          skMaxSleepBeforeThreadExit)) {
                        // all good go pick the operation
                    } else {
                        // end this thread it seems there is no work remaining
                        --m_data->m_idle_threads;
                        break;
                    }
                }

                // search for the next operation starting with the highest
                // priority Note: there has to be such operation as we acquired
                // the semaphore above
                //       so if popping fails spontaneously we are good to repeat
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
            }

            if (op) {
                if (trace_utils::is_debug_enabled() && last_label != label) {
                    thread_utils::set_current_thread_name(
                      s_bucket_labels[label]);
                    last_label = label;
                }

                execute_operation_on_this_thread(*op);
                op.reset();
            }
        }

        const auto remaining = --m_data->m_active_threads;
        XDISPATCH_TRACE() << "joining thread - " << remaining << " remaining";
        operation_queue_manager::instance().detach(this);
    }

private:
    threadpool::data_ptr m_data;
    std::thread m_thread;
};

threadpool::threadpool()
  : ithreadpool()
  , m_data(std::make_shared<data>())
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

std::shared_ptr<threadpool>
threadpool::instance()
{
    // this is an intentional leak so that the destructor is ok to run from
    // within a pool thread
    static auto* s_instance =
      new std::shared_ptr<threadpool>(std::make_shared<threadpool>());
    return *s_instance;
}

void
threadpool::schedule()
{
    // lets check if there is an idle thread first
    const int active_threads = m_data->m_active_threads;
    const int idle_threads = m_data->m_idle_threads;
    if (idle_threads > 0) {
        --m_data->m_idle_threads;
        XDISPATCH_TRACE() << "Waking one of " << idle_threads
                          << " idle threads (" << active_threads << " active)";
    }
    // check if we are good to create another thread
    else if (active_threads < m_data->m_max_threads) {
        auto thread = std::make_shared<worker>(m_data);
        operation_queue_manager::instance().attach(thread);
        ++m_data->m_active_threads;

        XDISPATCH_TRACE() << "spawned thread " << thread->get_id() << " ("
                          << (active_threads + 1) << "/"
                          << m_data->m_max_threads << ")";
    }
    // all threads busy and processor allocation reached, wait
    // and the operation will be picked up as soon as a thread is available
}

void
threadpool::notify_thread_blocked()
{
    const auto max_threads = ++m_data->m_max_threads;
    XDISPATCH_TRACE() << "increased threadcount to " << max_threads;
    schedule();
}

void
threadpool::notify_thread_unblocked()
{
    const auto max_threads = --m_data->m_max_threads;
    XDISPATCH_TRACE() << "lowered threadcount to " << max_threads;
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
