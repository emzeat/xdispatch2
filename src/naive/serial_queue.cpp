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

#include <thread>
#include <mutex>
#include <vector>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class serial_queue_impl : public iqueue_impl
{
public:
    serial_queue_impl(
        const ithread_ptr& thread
    ) : iqueue_impl()
        , m_thread( thread )
    {
        XDISPATCH_ASSERT( m_thread );
    }

    ~serial_queue_impl() final
    {
    }

    void async(
        const operation_ptr& op
    ) final
    {
        m_thread->execute( op );
    }

    void apply(
        size_t times,
        const iteration_operation_ptr& op
    ) final
    {
        const auto completed = std::make_shared< consumable >( times );
        for( size_t i = 0; i < times; ++i )
        {
            m_thread->execute( std::make_shared< apply_operation >( i, op, completed ) );
        }
        completed->waitForConsumed();
    }

    void after(
        std::chrono::milliseconds delay,
        const operation_ptr& op
    ) final
    {
        m_thread->execute( std::make_shared< delayed_operation >( delay, op ) );
    }

    backend_type backend() final
    {
        return backend_type::naive;
    }

private:
    ithread_ptr m_thread;
};

queue create_serial_queue(
    const std::string& label,
    const ithread_ptr& thread
)
{
    XDISPATCH_ASSERT( thread );
    return queue( label, std::make_shared< serial_queue_impl >( thread ) );
}

iqueue_impl_ptr backend::create_serial_queue(
    const std::string& /* label */
)
{
    return std::make_shared< serial_queue_impl >( std::make_shared< naive_thread >() );
}

}
__XDISPATCH_END_NAMESPACE
