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

#include <QtCore/QThreadPool>
#include <QtCore/QRunnable>
#include <QtCore/QPointer>

#include "qt5_backend_internal.h"
#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

class ThreadPoolProxy : public ithreadpool
{
public:
    ThreadPoolProxy(
        QThreadPool* pool,
        const std::string& label,
        queue_priority priority
    )
        : m_pool( pool )
        , m_label( label )
        , m_priority( priority )
    {
        XDISPATCH_ASSERT( pool );
    }

    ~ThreadPoolProxy()
    {
        if( m_pool )
        {
            m_pool->waitForDone();
        }
    }

    void execute(
        const operation_ptr& work,
        const queue_priority priority
    )
    {
        class OperationRunnable : public QRunnable
        {
        public:
            OperationRunnable(
                const operation_ptr& op,
                const std::string& label,
                queue_priority priority
            )
                : m_operation( op )
                , m_label( label )
                , m_priority( priority )
            {
                setAutoDelete( true );
            }

            void run() final
            {
                thread_utils::set_current_thread_name( m_label );
                thread_utils::set_current_thread_priority( m_priority );
                execute_operation_on_this_thread( *m_operation );
            }

        private:
            const operation_ptr m_operation;
            const std::string& m_label;
            const queue_priority m_priority;
        };

        int p = 0;
        switch( priority )
        {
        case queue_priority::BACKGROUND:
            p = 0;
            break;
        case queue_priority::DEFAULT:
        case queue_priority::UTILITY:
            p = 20;
            break;
        case queue_priority::USER_INITIATED:
            p = 40;
            break;
        case queue_priority::USER_INTERACTIVE:
            p = 100;
            break;
        }

        XDISPATCH_ASSERT( m_pool );
        m_pool->start( new OperationRunnable( work, m_label, m_priority ), p );
    }

private:
    QPointer<QThreadPool> m_pool;
    const std::string m_label;
    const queue_priority m_priority;
};

queue create_parallel_queue(
    const std::string& label,
    QThreadPool* pool,
    queue_priority priority
)
{
    XDISPATCH_ASSERT( pool );
    return naive::create_parallel_queue( label, std::make_shared< ThreadPoolProxy >( pool, label, priority ), priority, backend_type::qt5 );
}

iqueue_impl_ptr backend::create_parallel_queue(
    const std::string& label,
    queue_priority priority
)
{
    return qt5::create_parallel_queue( label, QThreadPool::globalInstance(), priority ).implementation();
}


}
__XDISPATCH_END_NAMESPACE
