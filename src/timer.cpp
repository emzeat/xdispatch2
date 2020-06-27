/*
* timer.cpp
*
* Copyright (c) 2011-2018 MLBA-Team
* All rights reserved.
*
* @LICENSE_HEADER_START@
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
* @LICENSE_HEADER_END@
*/

#include "xdispatch_internal.h"
#include "xdispatch/itimer_impl.h"
#include "xdispatch/iqueue_impl.h"

__XDISPATCH_USE_NAMESPACE

timer::timer(
    const itimer_impl_ptr& impl,
    const queue& target
)
    : m_impl( impl )
    , m_target_queue( target )
{
    XDISPATCH_ASSERT( m_impl );
}


void timer::interval(
    std::chrono::milliseconds interval
)
{
    m_impl->interval( interval );
}


void timer::latency(
    timer_precision precision
)
{
    m_impl->latency( precision );
}


void timer::start(
    std::chrono::milliseconds d
)
{
    m_impl->start( d );
}

void timer::handler(
    const operation_ptr& op
)
{
    m_impl->handler( op );
}

void timer::target_queue(
    const queue& q
)
{
    const auto q_impl = q.implementation();
    if( m_impl->backend() == q_impl->backend() )
    {
        m_impl->target_queue( q_impl );
    }
    else
    {
        throw std::runtime_error( "Cannot mix two different backends" );
    }

    m_target_queue = q;
}

queue timer::target_queue() const
{
    return m_target_queue;
}
