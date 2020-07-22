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

#include "naive_operations.h"
#include "../xdispatch_internal.h"

#include <thread>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

apply_operation::apply_operation(
    size_t index,
    const iteration_operation_ptr& op,
    const consumable_ptr& consumable
)
    : m_index( index )
    , m_op( op )
    , m_consumable( consumable )
{
}

void apply_operation::operator()()
{
    execute_operation_on_this_thread( *m_op, m_index );

    if( m_consumable )
    {
        m_consumable->consume();
    }
}

delayed_operation::delayed_operation(
    std::chrono::milliseconds delay,
    const operation_ptr& op,
    const consumable_ptr& consumable
)
    : m_delay( delay )
    , m_op( op )
    , m_consumable( consumable )
{

}

void delayed_operation::operator()()
{
    // FIXME(zwicker): This is very rough and may be blocking a thread - refine!
    std::this_thread::sleep_for( m_delay );

    execute_operation_on_this_thread( *m_op );

    if( m_consumable )
    {
        m_consumable->consume();
    }
}

consuming_operation::consuming_operation(
    const operation_ptr& op,
    const consumable_ptr& consumable
)
    : m_op( op )
    , m_consumable( consumable )
{

}

void consuming_operation::operator()()
{
    execute_operation_on_this_thread( *m_op );

    if( m_consumable )
    {
        m_consumable->consume();
    }
}

}
__XDISPATCH_END_NAMESPACE
