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

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QEvent>

#include "qt5_backend_internal.h"
#include "qt5_threadpool.h"
#include "../naive/naive_threadpool.h"
#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

class ExecuteOperationEvent : public QEvent
{
public:
    static QEvent::Type Type()
    {
        static int sType = QEvent::registerEventType();
        return static_cast<QEvent::Type>( sType );
    }

    ExecuteOperationEvent(
        const operation_ptr& work
    )
        : QEvent( Type() )
        , m_work( work )
    {
    }

    void execute()
    {
        execute_operation_on_this_thread( *m_work );
    }

private:
    const operation_ptr m_work;
};

class ThreadProxy : public naive::ithreadpool, public QObject
{
public:
    ThreadProxy(
        QThread* thread,
        const std::string& label,
        queue_priority priority
    )
        : m_thread( thread )
        , m_label( label )
        , m_priority( priority )
    {
        XDISPATCH_ASSERT( m_thread );
        moveToThread( m_thread );
    }

    ~ThreadProxy()
    {
    }

    void execute(
        const operation_ptr& work,
        queue_priority /* priority */
    ) final
    {
        QCoreApplication::postEvent( this, new ExecuteOperationEvent( work ) );
    }

protected:
    void customEvent(
        QEvent* event
    ) final
    {
        if( ExecuteOperationEvent::Type() == event->type() )
        {
            thread_utils::set_current_thread_name( m_label );
            thread_utils::set_current_thread_priority( m_priority );
            static_cast<ExecuteOperationEvent*>( event )->execute();
        }

        QObject::customEvent( event );
    }

private:
    QThread* m_thread;
    const std::string m_label;
    const queue_priority m_priority;
};

queue create_serial_queue(
    const std::string& label,
    QThread* thread,
    queue_priority priority
)
{
    XDISPATCH_ASSERT( thread );
    thread->setObjectName( QString::fromStdString( label ) );
    return naive::create_serial_queue( label, std::make_shared< ThreadProxy >( thread, label, priority ), priority, backend_type::qt5 );
}

iqueue_impl_ptr backend::create_serial_queue(
    const std::string& label,
    queue_priority priority
)
{
    return naive::create_serial_queue( label, naive::threadpool::instance(), priority, backend_type::qt5 ).implementation();
}

iqueue_impl_ptr backend::create_main_queue(
    const std::string& label
)
{
    auto instance = QCoreApplication::instance();
    if( nullptr == instance )
    {
        throw std::logic_error( "Construct a QCoreApplication before using the main queue" );
    }
    return qt5::create_serial_queue( label, instance->thread() ).implementation();
}

void backend::exec()
{
    auto instance = QCoreApplication::instance();
    if( nullptr == instance )
    {
        throw std::logic_error( "Construct a QCoreApplication before invoking exec()" );
    }
    instance->exec();
}

}
__XDISPATCH_END_NAMESPACE
