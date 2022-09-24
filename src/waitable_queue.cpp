/*
 * waitable_queue.cpp
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

#include <list>
#include <condition_variable>
#include <mutex>

#include "xdispatch_internal.h"
#include "xdispatch/waitable_queue.h"
#include "xdispatch/impl/iqueue_impl.h"
#include "xdispatch/impl/itimer_impl.h"
#include "trace_utils.h"
#include "naive/naive_operations.h"

__XDISPATCH_USE_NAMESPACE

class waitable_queue_operation : public operation
{
public:
    waitable_queue_operation()
      : m_CS()
      , m_cond()
      , m_operations()
      , m_completed(0)
      , m_active(false)
    {}

    void operator()() override { drain_one(); }

    void drain_one()
    {
        std::unique_lock<std::mutex> lock(m_CS);
        // wait for parallel executions to complete
        if (m_active) {
            m_cond.wait(lock, [this] { return !m_active; });
        }
        // we are the active worker now
        m_active = true;
        // try to pop and execute one operation
        if (!m_operations.empty()) {
            auto op = m_operations.front();
            m_operations.pop_front();

            lock.unlock();
            xdispatch::execute_operation_on_this_thread(*op);
            lock.lock();
            ++m_completed;
        }
        // notify we are no longer active
        m_active = false;
        m_cond.notify_all();
    }

    void wait_for_one()
    {
        std::unique_lock<std::mutex> lock(m_CS);
        wait_for_one(lock);
    }

    void wait_for_one(std::unique_lock<std::mutex>& lock)
    {
        while (0 == m_completed) {
            // when the queue is already active processing a queued operation
            // we can just wait for it to complete and will not deadlock
            if (m_active) {
                XDISPATCH_TRACE() << "Waiting for active operation";
                m_cond.wait(lock, [this] { return !m_active; });
            }
            // there is some chance nothing was queued to begin with
            else if (m_operations.empty()) {
                XDISPATCH_TRACE() << "No operations to wait for";
                return;
            }
            // when the queue is not active we can just take over and do the
            // processing as the queue would need to get scheduled first
            else {
                lock.unlock();
                XDISPATCH_TRACE()
                  << "Operation queue is starving, execute directly";
                drain_one();
                lock.lock();
            }
        }
        // decrement the completion counter
        --m_completed;
    }

    void wait_for_all()
    {
        std::unique_lock<std::mutex> lock(m_CS);
        while (m_active || !m_operations.empty()) {
            wait_for_one(lock);
        }
    }

    void add_one(const operation_ptr& op)
    {
        std::unique_lock<std::mutex> lock(m_CS);
        m_operations.push_back(op);
    }

private:
    std::mutex m_CS;
    std::condition_variable m_cond;

    std::list<operation_ptr> m_operations;
    size_t m_completed;
    bool m_active;
};

class waitable_queue::impl
  : public std::enable_shared_from_this<waitable_queue::impl>
  , public iqueue_impl
{
public:
    impl(const queue& inner_queue)
      : m_worker(std::make_shared<waitable_queue_operation>())
      , m_inner_queue(inner_queue)
    {}

    void wait_for_one() { m_worker->wait_for_one(); }

    void wait_for_all() { m_worker->wait_for_all(); }

    void async(const operation_ptr& op) override
    {
        m_worker->add_one(op);
        m_inner_queue.async(m_worker);
    }

    void apply(size_t times, const iteration_operation_ptr& op) override
    {
        for (size_t i = 0; i < times; ++i) {
            async(std::make_shared<naive::apply_operation>(i, op));
        }
        m_worker->wait_for_all();
    }

    void after(std::chrono::milliseconds delay,
               const operation_ptr& op) override
    {
        auto timer =
          backend_for_type(backend()).create_timer(shared_from_this());
        timer->handler(make_operation([op, timer] {
            execute_operation_on_this_thread(*op);
            timer->cancel();
        }));
        timer->resume(delay);
    }

    backend_type backend() override
    {
        return m_inner_queue.implementation()->backend();
    }

private:
    std::shared_ptr<waitable_queue_operation> m_worker;
    queue m_inner_queue;
};

waitable_queue::waitable_queue(const std::string& label,
                               queue_priority priority)
  : waitable_queue(label, queue(label, priority))
{}

waitable_queue::waitable_queue(const std::string& label,
                               const queue& inner_queue)
  : queue(label, std::make_shared<impl>(inner_queue))
{}

void
waitable_queue::wait_for_one()
{
    const auto inner = std::static_pointer_cast<impl>(implementation());
    return inner->wait_for_one();
}

void
waitable_queue::wait_for_all()
{
    const auto inner = std::static_pointer_cast<impl>(implementation());
    return inner->wait_for_all();
}
