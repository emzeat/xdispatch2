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

#include "naive_threadpool.h"
#include "naive_trace.h"
#include "naive_inverse_lockguard.h"

#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

threadpool::threadpool()
    : ithreadpool()
    , m_CS()
    , m_max_threads( thread_utils::system_thread_count() )
    , m_threads()
    , m_idle_threads( 0 )
    , m_operations()
    , m_cancelled( false )
{
    XDISPATCH_TRACE() << "threadpool with " << m_max_threads << " ideal threads" << std::endl;
}

threadpool::~threadpool()
{
    {
        std::lock_guard<std::mutex> lock( m_CS );
        m_cancelled = true;
        m_cond.notify_all();
    }

    for( const auto& thread : m_threads )
    {
        XDISPATCH_ASSERT( thread->joinable() && "Thread should not delete itself" );
        thread->join();
    }
}

void threadpool::execute(
    const operation_ptr& work,
    const queue_priority /* priority */
)
{
    std::lock_guard<std::mutex> lock( m_CS );
    m_operations.push_back( work );
    schedule();
}

void threadpool::thread_blocked()
{
    std::lock_guard<std::mutex> lock( m_CS );
    ++m_max_threads;
    XDISPATCH_TRACE() << "increased threads to " << m_max_threads << std::endl;
    schedule();
}

void threadpool::thread_unblocked()
{
    std::lock_guard<std::mutex> lock( m_CS );
    --m_max_threads;
    XDISPATCH_TRACE() << "lowered threads again to " << m_max_threads << std::endl;
    schedule();
}

ithreadpool_ptr threadpool::instance()
{
    static ithreadpool_ptr s_instance = std::make_shared< threadpool >();
    return s_instance;
}

void threadpool::schedule()
{
    // lets check if there is an idle thread first
    const auto active_threads = m_threads.size();
    if( 0 != m_idle_threads )
    {
        --m_idle_threads;
        m_cond.notify_one();
    }
    // check if we are good to create another thread
    else if( active_threads < m_max_threads )
    {
        XDISPATCH_TRACE() << "spawning thread #" << ( active_threads + 1 )
                          << " of " << m_max_threads << std::endl;

        auto thread = std::make_shared< std::thread >( &threadpool::run_thread, this );
        m_threads.push_back( std::move( thread ) );
    }
    // all threads busy and processor allocation reached, wait
    // and the operation will be picked up as soon as a thread is available
    else
    {
        XDISPATCH_TRACE() << "fully loaded - threads=" << active_threads
                          << ", idle=" << m_idle_threads << std::endl;
    }
}

void threadpool::run_thread()
{
    std::unique_lock<std::mutex> lock( m_CS );

    while( !m_cancelled )
    {
        operation_ptr op;
        {
            if( m_operations.empty() )
            {
                ++m_idle_threads;
                m_cond.wait( lock );
            }
            if( !m_operations.empty() )
            {
                op = m_operations.front();
                m_operations.pop_front();
            }
        }

        inverse_lock_guard< std::mutex > unlock( m_CS );
        if( op )
        {
            execute_operation_on_this_thread( *op );
            op.reset();
        }
    }

    XDISPATCH_TRACE() << "joining thread" << std::endl;
}

}
__XDISPATCH_END_NAMESPACE
