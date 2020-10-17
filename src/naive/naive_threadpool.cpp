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
    , m_max_threads( 0 )
    , m_threads()
    , m_idle_threads( 0 )
    , m_operations()
    , m_cancelled( false )
{
    // we are overcommitting by default so that it becomes less likely
    // that operations get starved due to threads blocking on resources
    m_max_threads = 2 * thread_utils::system_thread_count();
    XDISPATCH_TRACE() << "threadpool with " << m_max_threads << " system threads" << std::endl;
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
    const queue_priority priority
)
{
    std::lock_guard<std::mutex> lock( m_CS );
    int index = -1;
    switch( priority )
    {
    case queue_priority::USER_INTERACTIVE:
        index = 0;
        break;
    case queue_priority::USER_INITIATED:
        index = 1;
        break;
    case queue_priority::UTILITY:
    case queue_priority::DEFAULT:
    default:
        index = 2;
        break;
    case queue_priority::BACKGROUND:
        index = 3;
        break;
    }

    XDISPATCH_ASSERT( index >= 0 );
    m_operations[index].push_back( work );
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
        //        XDISPATCH_TRACE() << "fully loaded - threads=" << active_threads
        //                          << ", idle=" << m_idle_threads << std::endl;
    }
}

void threadpool::run_thread()
{
    std::unique_lock<std::mutex> lock( m_CS );

    while( !m_cancelled )
    {
        operation_ptr op;
        {
            // search for the next operation starting with the highest priority
            for( auto& ops_prio : m_operations )
            {
                if( !ops_prio.empty() )
                {
                    op = ops_prio.front();
                    ops_prio.pop_front();
                    break;
                }
            }
            // suspend when no operations are queued
            if( !op )
            {
                ++m_idle_threads;
                m_cond.wait( lock );
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
