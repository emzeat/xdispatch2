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
#include "../trace_utils.h"
#include "naive_inverse_lockguard.h"

#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

char const* const s_bucket_labels[ threadpool::bucket_count ] =
{
    k_label_global_INTERACTIVE,
    k_label_global_INITIATED,
    k_label_global_UTILITY,
    k_label_global_BACKGROUND
};

threadpool::threadpool()
    : ithreadpool()
    , m_operations_counter( 0 )
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
    m_cancelled = true;
    m_operations_counter.release( m_threads.size() );

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
    int index = -1;
    switch( priority )
    {
    case queue_priority::USER_INTERACTIVE:
        index = bucket_USER_INTERACTIVE;
        break;
    case queue_priority::USER_INITIATED:
        index = bucket_USER_INITIATED;
        break;
    case queue_priority::UTILITY:
    case queue_priority::DEFAULT:
        index = bucket_UTILITY;
        break;
    case queue_priority::BACKGROUND:
        index = bucket_BACKGROUND;
        break;
    }

    XDISPATCH_ASSERT( index >= 0 );
    const auto enqueued = m_operations[index].enqueue( work );
    XDISPATCH_ASSERT( enqueued );
    schedule();
}

std::shared_ptr< threadpool > threadpool::instance()
{
    // this is an intentional leak so that the destructor is ok to run from within a pool thread
    static auto* s_instance = new std::shared_ptr< threadpool >( std::make_shared< threadpool >() );
    return *s_instance;
}

void threadpool::schedule()
{
    // increment the semaphore
    m_operations_counter.release();

    // lets check if there is an idle thread first
    const int active_threads = m_threads.size();
    const int idle_threads = m_idle_threads;
    if( 0 != idle_threads )
    {
        --m_idle_threads;
        XDISPATCH_TRACE() << "Waking one of " << idle_threads << " idle threads" << std::endl;
    }
    // check if we are good to create another thread
    else if( active_threads < m_max_threads )
    {
        auto thread = std::make_shared< std::thread >( &threadpool::run_thread, this );

        XDISPATCH_TRACE() << "spawned thread " << thread->get_id()
                          << " (" << ( active_threads + 1 ) << "/" << m_max_threads << ")" << std::endl;
        m_threads.push_back( std::move( thread ) );
    }
    // all threads busy and processor allocation reached, wait
    // and the operation will be picked up as soon as a thread is available
}

void threadpool::run_thread()
{
    int last_label = -1;
    while( !m_cancelled )
    {
        operation_ptr op;
        int label = -1;
        {
            // if no op we are idling and need to block on our op counter
            if( m_operations_counter.try_acquire() )
            {
            }
            else
            {
                XDISPATCH_TRACE() << std::this_thread::get_id() << " idling" << std::endl;

                ++m_idle_threads;
                m_operations_counter.acquire();
            }

            // search for the next operation starting with the highest priority
            for( label = 0; label < bucket_count; ++label )
            {
                auto& ops_prio = m_operations[ label ];
                if( ops_prio.try_dequeue( op ) )
                {
                    break;
                }
            }
        }

        if( op )
        {
            if( trace_utils::is_debug_enabled() && last_label != label )
            {
                thread_utils::set_current_thread_name( s_bucket_labels[label] );
                last_label = label;
            }

            execute_operation_on_this_thread( *op );
            op.reset();
        }
    }

    XDISPATCH_TRACE() << "joining thread" << std::endl;
}

void threadpool::notify_thread_blocked()
{
    const auto max_threads = ++m_max_threads;
    XDISPATCH_TRACE() << "increased threadcount to " << max_threads << std::endl;
    // FIXME(zwicker) This may increase the number of threads permanently
    //                as threads will never be joined again right now no matter
    //                how long they have been idle
    schedule();
}

void threadpool::notify_thread_unblocked()
{
    const auto max_threads = --m_max_threads;
    XDISPATCH_TRACE() << "lowered threadcount to " << max_threads << std::endl;
}

}
__XDISPATCH_END_NAMESPACE
