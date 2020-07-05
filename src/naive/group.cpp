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

#include "xdispatch/igroup_impl.h"
#include "xdispatch/iqueue_impl.h"

#include "backend_internal.h"
#include "consumable.h"
#include "operations.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class group_impl : public igroup_impl, public std::enable_shared_from_this< group_impl >
{
public:
    group_impl(
        backend_type backend
    )
        : igroup_impl()
        , m_backend( backend )
        , m_consumable( std::make_shared<consumable>() )
    {
    }

    ~group_impl() final
    {
    }

    void async(
        const operation_ptr& op,
        const iqueue_impl_ptr& q
    ) final
    {
        const auto c = std::atomic_load( &m_consumable );
        XDISPATCH_ASSERT( c );
        c->increment();
        q->async( std::make_shared<consuming_operation>( op, c ) );
    }

    bool wait(
        std::chrono::milliseconds timeout
    ) final
    {
        // swap the previous consumable with a new one that all operations
        // submitted after this call will be added and which waits on the
        // previous consumable in a chain. Use a compare/exchange and retry
        // whenver the consumable was already swapped by another thread
        consumable_ptr old_c;
        consumable_ptr new_c;
        do
        {
            old_c = std::atomic_load( &m_consumable );
            new_c = std::make_shared< consumable >( 0, old_c );
        }
        while( !std::atomic_compare_exchange_weak( &m_consumable, &old_c, new_c ) );
        XDISPATCH_ASSERT( old_c );
        XDISPATCH_ASSERT( new_c );
        return old_c->waitForConsumed( timeout );

        // FIXME(zwicker): This is blocking and will not work if invoked from within
        //                 an operation active on the same queue as one of the operations
        //                 listed in the consumable
    }

    void notify(
        const operation_ptr& op,
        const iqueue_impl_ptr& q
    ) final
    {
        // FIXME(zwicker): This will be creating a temporary queue for no good reason
        //                 and can probably be optimized to share an existing thread
        const auto this_ptr = shared_from_this();
        const auto notify_q = create_serial_queue( k_label_global_low + std::string( "_notify" ), queue_priority::UTILITY );
        notify_q.async( [op, q, this_ptr]
        {
            this_ptr->wait( std::chrono::milliseconds::max() );
            q->async( op );
        } );
    }

    backend_type backend() final
    {
        return m_backend;
    }

private:
    const backend_type m_backend;
    consumable_ptr m_consumable;
};

igroup_impl_ptr backend::create_group(
    backend_type backend
)
{
    return std::make_shared< group_impl >( backend );
}

}
__XDISPATCH_END_NAMESPACE
