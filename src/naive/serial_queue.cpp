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
#include "operation_queue.h"
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
        , m_queue( std::make_shared< operation_queue >( thread ) )
    {
        XDISPATCH_ASSERT( thread );
        m_queue->attach();
    }

    ~serial_queue_impl() final
    {
        m_queue->detach();
    }

    void async(
        const operation_ptr& op
    ) final
    {
        m_queue->async( op );
    }

    void apply(
        size_t times,
        const iteration_operation_ptr& op
    ) final
    {
        const auto completed = std::make_shared< consumable >( times );
        for( size_t i = 0; i < times; ++i )
        {
            async( std::make_shared< apply_operation >( i, op, completed ) );
        }
        completed->waitForConsumed();

        // FIXME(zwicker): This is blocking and will not work if invoked from within
        //                 an operation active on this very same queue
    }

    void after(
        std::chrono::milliseconds delay,
        const operation_ptr& op
    ) final
    {
        async( std::make_shared< delayed_operation >( delay, op ) );
    }

    backend_type backend() final
    {
        return backend_type::naive;
    }

private:
    operation_queue_ptr m_queue;
};

queue create_serial_queue(
    const std::string& label,
    const ithread_ptr& thread
)
{
    if( thread )
    {
        return queue( label, std::make_shared< serial_queue_impl >( thread ) );
    }
    return queue( label, std::make_shared< serial_queue_impl >( std::make_shared< naive_thread >( label ) ) );
}

iqueue_impl_ptr backend::create_serial_queue(
    const std::string& label
)
{
    return std::make_shared< serial_queue_impl >( std::make_shared< naive_thread >( label ) );
}

static std::shared_ptr<manual_thread> main_thread()
{
    static std::shared_ptr<manual_thread> s_thread = std::make_shared< manual_thread >( k_label_main );
    return s_thread;
}

iqueue_impl_ptr backend::create_main_queue(
    const std::string& /* label */
)
{
    static iqueue_impl_ptr s_queue = std::make_shared< serial_queue_impl >( main_thread() );
    return s_queue;
}

void backend::exec()
{
    main_thread()->drain();
}

}
__XDISPATCH_END_NAMESPACE
