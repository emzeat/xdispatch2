/*
* barrier_operation.cpp
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
#include "xdispatch/barrier_operation.h"

__XDISPATCH_BEGIN_NAMESPACE

barrier_operation::barrier_operation()
    : operation()
    , m_should_wait( true )
    , m_mutex()
    , m_cond()
{

}

bool barrier_operation::wait(
    std::chrono::milliseconds timeout
)
{
    std::unique_lock< std::mutex > lock( m_mutex );
    if( m_should_wait )
    {
        if( std::chrono::milliseconds( -1 ) != timeout )
        {
            return m_cond.wait_for( lock, timeout, [this]{
                return !m_should_wait;
            } );
        }
        else
        {
            m_cond.wait( lock, [this]{
                return !m_should_wait;
            } );
            return true;
        }
    }
    else
    {
        return true;
    }
}

bool barrier_operation::has_passed() const
{
    std::unique_lock< std::mutex > lock( m_mutex );
    return false == m_should_wait;
}

void barrier_operation::operator()()
{
    std::unique_lock< std::mutex > lock( m_mutex );
    m_should_wait = false;
    m_cond.notify_all();
}

__XDISPATCH_END_NAMESPACE
