/*
* backend_libdispatch.h
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


#ifndef XDISPATCH_BACKEND_QT5_H_
#define XDISPATCH_BACKEND_QT5_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include <QtCore/QThread>
#include <QtCore/QThreadPool>

#include "xdispatch/dispatch.h"
#include "xdispatch/signals.h"
#if (!BUILD_XDISPATCH2_BACKEND_QT5)
    #error "The qt5 backend is not available on this platform"
#endif

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

/**
    @return A new serial queue using the given thread

    @param label The name to use for the new queue
    @param thread The thread on which all operations added to the queue
                  will be executed
    @param priority Choose a priority different from default to automatically
                  have the priority of the thread reconfigured
    */
XDISPATCH_EXPORT queue
create_serial_queue(
    const std::string& label,
    QThread* thread,
    queue_priority priority = queue_priority::DEFAULT
);

/**
    @return A new parallel queue powered by the given pool

    @remarks Make sure not to destroy the pool from an operation
             running on the same pool as QThreadPool blocks in the
             destructor until all active tasks have completed which
             will cause a deadlock

    @param label The name to use for the new queue
    @param pool The threadpool on which queued operations will be executed
    @param priority Controls the priority assigned to draining the queue
                relative from other runnables added to the pool
    */
XDISPATCH_EXPORT queue
create_parallel_queue(
    const std::string& label,
    QThreadPool* pool,
    queue_priority priority = queue_priority::DEFAULT
);

/**
    @brief Registers the given connection with object

    The connection will be automatically closed when
    the object gets deleted

    @param object The object to tie the lifetime of the connection to
    @param connection The connection to be managed
 */
XDISPATCH_EXPORT void register_connection(
    QObject* object,
    const connection& connection
);

/**
    @brief Destroys all connections with object

    @param object The object to to remove connections from
    @param signal The signal to which connections get removed
 */
XDISPATCH_EXPORT void destroy_connections(
    QObject* object,
    signal_p& signal
);

/**
    @brief helper to connect a QObject slot to an xdispatch::signal

    Will automatically take care of closing the connection when the
    object gets destroyed

    @param sender The signal to connect to
    @param receiver The object on which the slot is to be called
    @param slot The slot on object to be called
    @param q The queue to invoke the slot on
    @param m Selects the notification mode, useful for high frequency updates
 */
template<class Object, typename...Args, typename... SlotArgs>
void connect(
    signal<void( Args... )>& sender,
    Object* receiver,
    void( Object::*slot )( SlotArgs... ),
    const queue& q = main_queue(),
    notification_mode m = notification_mode::single_updates
)
{
    register_connection( receiver, sender.template connect<Object, SlotArgs...>( receiver, slot, q, m ) );
}

/**
    @brief helper to connect a lambda to an xdispatch::signal

    Will automatically take care of closing the connection when the
    accompanying object gets destroyed

    @param sender The signal to connect to
    @param receiver The object controlling the lifetime of the connection
    @param lambda The lambda to be called
    @param q The queue to invoke the slot on
    @param m Selects the notification mode, useful for high frequency updates
 */
template<typename... Args>
void connect(
    signal<void( Args... )>& sender,
    QObject* receiver,
    const typename signal<void( Args... )>::functor& lambda,
    const queue& q = main_queue(),
    notification_mode m = notification_mode::single_updates
)
{
    register_connection( receiver, sender.connect( lambda, q, m ) );
}

/**
    @brief helper to disconnect an object from a signal

    Use this to explicitly destroy all connections
    made between an object and a signal

    @param sender The signal to disconnect from
    @param receiver The object for which to remove connections
 */
template<typename... Args>
void disconnect(
    signal<void( Args... )>& sender,
    QObject* receiver
)
{
    destroy_connections( receiver, sender );
}

}
__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_BACKEND_QT5_H_ */
