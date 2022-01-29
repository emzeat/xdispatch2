/*
 * naive_parallel_queue.cpp
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

#include "xdispatch/iqueue_impl.h"

#include "naive_backend_internal.h"
#include "naive_thread.h"
#include "naive_threadpool.h"
#include "naive_operation_queue_manager.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class parallel_queue_impl : public iqueue_impl
{
public:
    parallel_queue_impl(const ithreadpool_ptr& pool,
                        const queue_priority priority,
                        backend_type backend)
      : iqueue_impl()
      , m_backend(backend)
      , m_pool(pool)
      , m_priority(priority)
    {
        XDISPATCH_ASSERT(m_pool);
        operation_queue_manager::instance().attach(m_pool);
    }

    ~parallel_queue_impl() override
    {
        operation_queue_manager::instance().detach(m_pool.get());
    }

    void async(const operation_ptr& op) final
    {
        m_pool->execute(op, m_priority);
    }

    void apply(size_t times, const iteration_operation_ptr& op) final
    {
        const auto completed = std::make_shared<consumable>(times);
        for (size_t i = 0; i < times; ++i) {
            async(std::make_shared<apply_operation>(i, op, completed));
        }
        completed->wait_for_consumed();
    }

    void after(std::chrono::milliseconds delay, const operation_ptr& op) final
    {
        async(std::make_shared<delayed_operation>(delay, op));
    }

    backend_type backend() final { return m_backend; }

private:
    const backend_type m_backend;
    ithreadpool_ptr m_pool;
    const queue_priority m_priority;
};

queue
create_parallel_queue(const std::string& label,
                      const ithreadpool_ptr& pool,
                      queue_priority priority)
{
    return create_parallel_queue(label, pool, priority, backend_type::naive);
}

queue
create_parallel_queue(const std::string& label,
                      const ithreadpool_ptr& pool,
                      queue_priority priority,
                      backend_type backend)
{
    XDISPATCH_ASSERT(pool);
    return queue(
      label, std::make_shared<parallel_queue_impl>(pool, priority, backend));
}

iqueue_impl_ptr
backend::create_parallel_queue(const std::string& /*label*/,
                               queue_priority priority,
                               backend_type backend)
{
    return std::make_shared<parallel_queue_impl>(
      threadpool::instance(), priority, backend);
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
