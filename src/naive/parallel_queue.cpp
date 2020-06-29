/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/

#include "xdispatch/iqueue_impl.h"

#include "backend_internal.h"
#include "naive_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class parallel_queue_impl : public iqueue_impl
{
public:
    parallel_queue_impl(
        const ithreadpool_ptr& pool,
        const queue_priority priority
    ) : iqueue_impl()
        , m_pool( pool )
        , m_priority( priority )
    {
        XDISPATCH_ASSERT( m_pool );
    }

    ~parallel_queue_impl() final
    {
    }

    void async(
        const operation_ptr& op
    ) final
    {
        m_pool->execute( op, m_priority );
    }

    void apply(
        size_t times,
        const iteration_operation_ptr& op
    ) final
    {
        const auto completed = std::make_shared< consumable >( times );
        for( size_t i = 0; i < times; ++i )
        {
            m_pool->execute( std::make_shared< apply_operation >( i, op, completed ), m_priority );
        }
        completed->waitForConsumed();
    }

    void after(
        std::chrono::milliseconds delay,
        const operation_ptr& op
    ) final
    {
        m_pool->execute( std::make_shared< delayed_operation >( delay, op ), m_priority );
    }

    backend_type backend() final
    {
        return backend_type::naive;
    }

private:
    ithreadpool_ptr m_pool;
    const queue_priority m_priority;
};

queue create_parallel_queue(
    const std::string& label,
    const ithreadpool_ptr& pool,
    queue_priority priority
)
{
    XDISPATCH_ASSERT( pool );
    return queue( label, std::make_shared< parallel_queue_impl >( pool, priority ) );
}

iqueue_impl_ptr backend::create_parallel_queue(
    const std::string& /* label */,
    const queue_priority& priority
)
{
    return std::make_shared< parallel_queue_impl >( std::make_shared< naive_thread >(), priority );
}


}
__XDISPATCH_END_NAMESPACE
