/*
 * qt5_serial_queue.cpp
 *
 * Copyright (c) 2011 - 2022 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <QtCore/QEvent>

#include "qt5_backend_internal.h"
#include "qt5_threadpool.h"
#include "../naive/naive_threadpool.h"
#include "../trace_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5 {

class ExecuteOperationEvent : public QEvent
{
public:
    static QEvent::Type Type()
    {
        static int sType = QEvent::registerEventType();
        return static_cast<QEvent::Type>(sType);
    }

    ExecuteOperationEvent(const operation_ptr& work)
      : QEvent(Type())
      , m_work(work)
    {}

    void execute() { execute_operation_on_this_thread(*m_work); }

private:
    const operation_ptr m_work;
};

class ThreadProxy
  : public naive::ithreadpool
  , public QObject
{
public:
    explicit ThreadProxy(QThread* thread)
      : m_thread(thread)
    {
        XDISPATCH_ASSERT(m_thread);
        moveToThread(m_thread);
    }

    ~ThreadProxy() override = default;
    void execute(const operation_ptr& work, queue_priority /* priority */
                 ) final
    {
        QCoreApplication::postEvent(this, new ExecuteOperationEvent(work));
    }

    void notify_thread_blocked() final {}
    void notify_thread_unblocked() final {}

protected:
    void customEvent(QEvent* event) final
    {
        if (ExecuteOperationEvent::Type() == event->type()) {
            static_cast<ExecuteOperationEvent*>(event)->execute();
        }

        QObject::customEvent(event);
    }

private:
    QThread* m_thread;
};

queue
create_serial_queue(const std::string& label,
                    QThread* thread,
                    queue_priority priority)
{
    XDISPATCH_ASSERT(thread);
    thread->setObjectName(QString::fromStdString(label));
    auto proxy = std::make_shared<ThreadProxy>(thread);
    return naive::create_serial_queue(
      label, std::move(proxy), priority, backend_type::qt5);
}

iqueue_impl_ptr
backend::create_main_queue(const std::string& label)
{
    auto* instance = QCoreApplication::instance();
    if (nullptr == instance) {
        XDISPATCH_WARNING()
          << "No QCoreApplication found, main_queue() will fall "
             "back to native implementation";
        return backend_base::create_main_queue(label);
    }
    return qt5::create_serial_queue(label, instance->thread()).implementation();
}

void
backend::exec()
{
    auto* instance = QCoreApplication::instance();
    if (nullptr == instance) {
        XDISPATCH_WARNING()
          << "No QCoreApplication found, main_queue() will fall "
             "back to native implementation";
        return backend_base::exec();
    }
    instance->exec();
}

} // namespace qt5
__XDISPATCH_END_NAMESPACE
