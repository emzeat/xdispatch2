/*
 * naive_operation_queue.cpp
 *
 * Copyright (c) 2012 - 2022 Marius Zwicker
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

#include "naive_operation_queue.h"
#include "naive_operation_queue_manager.h"
#include "naive_thread.h"
#include "naive_inverse_lockguard.h"

#include "../thread_utils.h"
#include "../trace_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

operation_queue::operation_queue(const ithreadpool_ptr& threadpool,
                                 const std::string& label,
                                 queue_priority priority)
  : m_label(label)
  , m_priority(priority)
  , m_jobs()
  , m_CS()
  , m_active_drain(false)
  , m_notify_operation()
  , m_threadpool(threadpool)
{}

// helper to introduce a delay into a loop condition
inline bool
yield_drain()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}

operation_queue::~operation_queue()
{
    // ensure no new notifications can get queued
    // wait for calls to drain() to return so that
    // no dangling pointer access may happen on a different
    // thread.
    bool active_drain = false;
    do {
        std::lock_guard<std::mutex> lock(m_CS);
        m_notify_operation.reset();
        active_drain = m_active_drain;
    }
    // delay to give other threads a chance to proceed
    // if the drain is active
    while (active_drain && yield_drain());

    // release the threadpool
    m_threadpool.reset();
}

class drain_scope
{
public:
    drain_scope(bool& active_drain)
      : m_active_drain(active_drain)
    {
        m_active_drain = true;
    }
    drain_scope(const drain_scope&) = delete;

    ~drain_scope() { m_active_drain = false; }

private:
    bool& m_active_drain;
};

class deferred_pop
{
public:
    explicit deferred_pop(std::list<operation_ptr>& list)
      : m_list(list)
    {}
    deferred_pop(const deferred_pop&) = delete;

    ~deferred_pop() { m_list.pop_front(); }

private:
    std::list<operation_ptr>& m_list;
};

void
operation_queue::drain()
{
    if (trace_utils::is_debug_enabled()) {
        thread_utils::set_current_thread_name(m_label);
    }

    std::lock_guard<std::mutex> lock(m_CS);
    drain_scope scope(m_active_drain);
    // we need to satisfy two constraints here:
    // 1. do not remove the entry from m_jobs
    //    until AFTER it has been executed so that async()
    //    can test if all operations in m_jobs have COMPLETED
    //    by checking if m_jobs is empty
    // 2. make sure not to free an operation while m_CS is
    //    locked so that recursive scenarios are supported
    while (!m_jobs.empty()) {
        operation_ptr job;
        deferred_pop pop(m_jobs);
        std::swap(m_jobs.front(), job);
        {
            inverse_lock_guard<std::mutex> unlock(m_CS);
            if (job) {
                process_job(*job);
                job.reset();
            }
        }
    }
}

void
operation_queue::async(const operation_ptr& job)
{
    // preallocate outside the lock
    operation_ptr job2 = job;

    std::lock_guard<std::mutex> lock(m_CS);
    // we only need to notify, i.e. wake the thread
    // if all previous jobs have been COMPLETED. Elsewise
    // the thread is awake anyways and we can spare the overhead
    const bool notify = m_jobs.empty();
    m_jobs.push_back(std::move(job2));
    if (notify && m_notify_operation) {
        XDISPATCH_TRACE() << "'" << m_label << "' wake threadpool";
        m_threadpool->execute(m_notify_operation, m_priority);
    } else if (m_notify_operation) {
        XDISPATCH_TRACE() << "'" << m_label << "' already awake ("
                          << m_jobs.size() << ")";
    } else {
        XDISPATCH_WARNING()
          << "'" << m_label << "' detached, dropping operation";
    }
}

void
operation_queue::attach()
{
    std::lock_guard<std::mutex> lock(m_CS);
    m_notify_operation = make_operation(this, &operation_queue::drain);

    const auto this_ptr = shared_from_this();
    XDISPATCH_ASSERT(this_ptr);
    operation_queue_manager::instance().attach(this_ptr);
}

void
operation_queue::detach()
{
    bool empty = false;
    {
        std::lock_guard<std::mutex> lock(m_CS);
        empty = m_jobs.empty();
        m_notify_operation.reset();
    }

    if (empty) {
        // if there was no jobs remaining at the time the notify
        // operation was reset, there is no chance anymore for an
        // operation to be dispatched. As such we can cut it short
        // and skip the wakeup of our thread, directly unregistering
        // with the manager instead as no barrier is needed
        operation_queue_manager::instance().detach(this);
    } else {
        // queue a final operation which will be executed after
        // all others which have been queued so far. The final
        // operation will make sure to unregister with the queue
        // manager and hence release the operation_queue
        const auto detach_op = make_operation(
          [this] { operation_queue_manager::instance().detach(this); });
        async(detach_op);
    }
}

void
operation_queue::process_job(operation& job)
{
#if !(defined DEBUG)
    try
#endif
    {
        execute_operation_on_this_thread(job);
    }
#if !(defined DEBUG)
    catch (const std::exception& e) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before:\n"
                  << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;

        throw;
    } catch (...) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before!"
                  << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;

        std::terminate();
    }
#endif
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
