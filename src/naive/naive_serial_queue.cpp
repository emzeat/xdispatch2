/*
 * naive_serial_queue.cpp
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

#include "xdispatch/impl/iqueue_impl.h"

#include "naive_backend_internal.h"
#include "naive_operation_queue.h"
#include "naive_threadpool.h"

#include <thread>
#include <mutex>
#include <vector>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class serial_queue_impl
  : public std::enable_shared_from_this<serial_queue_impl>
  , public iqueue_impl
{
public:
    serial_queue_impl(const ithreadpool_ptr& threadpool,
                      const std::string& label,
                      queue_priority priority,
                      backend_type backend)
      : iqueue_impl()
      , m_backend(backend)
      , m_queue(std::make_shared<operation_queue>(threadpool, label, priority))
    {
        XDISPATCH_ASSERT(threadpool);
        m_queue->attach();
    }

    ~serial_queue_impl() override { m_queue->detach(); }

    void async(const queued_operation& op) final { m_queue->async(op); }

    void apply(size_t times, const iteration_operation_ptr& op) final
    {
        const auto completed = std::make_shared<consumable>(times);
        for (size_t i = 0; i < times; ++i) {
            operation_ptr operation =
              std::make_shared<apply_operation>(i, op, completed);
            async(std::move(operation));
        }
        completed->wait_for_consumed();

        // FIXME(zwicker): This is blocking and will not work if invoked from
        // within
        //                 an operation active on this very same queue
    }

    void after(std::chrono::milliseconds delay,
               const queued_operation& op) final
    {
        auto timer =
          backend_for_type(m_backend).create_timer(shared_from_this());
        delayed_operation::create_and_dispatch(std::move(timer), delay, op);
    }

    backend_type backend() final { return m_backend; }

private:
    const backend_type m_backend;
    operation_queue_ptr m_queue;
};

queue
create_serial_queue(const std::string& label,
                    const ithreadpool_ptr& thread,
                    queue_priority priority,
                    backend_type backend)
{
    XDISPATCH_ASSERT(thread);
    return queue(
      label,
      std::make_shared<serial_queue_impl>(thread, label, priority, backend));
}

queue
create_serial_queue(const std::string& label,
                    queue_priority priority,
                    backend_type backend)
{
    return queue(label,
                 std::make_shared<serial_queue_impl>(
                   threadpool::instance(), label, priority, backend));
}

queue
create_serial_queue(const std::string& label,
                    const ithreadpool_ptr& thread,
                    queue_priority priority)
{
    return create_serial_queue(label, thread, priority, backend_type::naive);
}

iqueue_impl_ptr
backend::create_serial_queue(const std::string& label,
                             queue_priority priority,
                             backend_type backend)
{
    return std::make_shared<serial_queue_impl>(
      threadpool::instance(), label, priority, backend);
}

static std::shared_ptr<manual_thread>
main_thread()
{
    static std::shared_ptr<manual_thread> s_thread =
      std::make_shared<manual_thread>(k_label_main,
                                      queue_priority::USER_INTERACTIVE);
    return s_thread;
}

iqueue_impl_ptr
backend::create_main_queue(const std::string& label, backend_type backend)
{
    static iqueue_impl_ptr s_queue = std::make_shared<serial_queue_impl>(
      main_thread(), label, queue_priority::USER_INTERACTIVE, backend);
    return s_queue;
}

void
backend::exec()
{
    main_thread()->run();
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
