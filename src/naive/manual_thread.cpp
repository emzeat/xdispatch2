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

#include "manual_thread.h"
#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

manual_thread::manual_thread(
    const std::string& name
)
    : ithread()
    , m_name( name )
    , m_cancelled( false )
{
}

manual_thread::~manual_thread()
{
}

void manual_thread::execute(
    const operation_ptr& work
)
{
    std::lock_guard<std::mutex> guard( m_CS );
    m_ops.push_back( work );
    m_cond.notify_all();
}

void manual_thread::drain()
{
    thread_utils::set_current_thread_name( m_name );
    while( !m_cancelled )
    {
        std::vector< operation_ptr > ops;
        {
            std::unique_lock<std::mutex> guard( m_CS );
            if( m_ops.empty() )
            {
                m_cond.wait( guard );
            }
            std::swap( m_ops, ops );
        }

        for( const auto& op : ops )
        {
            op->operator()();
        }
    }
}

void manual_thread::cancel()
{
    std::lock_guard<std::mutex> guard( m_CS );
    m_cancelled = true;
    m_cond.notify_all();
}

}
__XDISPATCH_END_NAMESPACE
