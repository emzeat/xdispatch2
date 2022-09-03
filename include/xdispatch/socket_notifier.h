/*
 * socket_notifier.h
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

#ifndef XDISPATCH_SOCKET_NOTIFIER_H_
#define XDISPATCH_SOCKET_NOTIFIER_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#ifndef __XDISPATCH_INDIRECT__
    #error                                                                     \
      "Please #include <xdispatch/dispatch.h> instead of this file directly."
    #include "dispatch.h"
#endif

__XDISPATCH_BEGIN_NAMESPACE

class isocket_notifier_impl;
using isocket_notifier_impl_ptr = std::shared_ptr<isocket_notifier_impl>;

/**
    @brief Describes a socket
 */
using socket_t = int;

/**
    @brief Indicates the type of operations tracked by the notifier
 */
enum class notifier_type
{
    READ,
    WRITE
};

/**
    @brief The operation executed when the socket is ready
 */
using socket_notifier_operation =
  parameterized_operation<socket_t, notifier_type>;

using socket_notifier_operation_ptr =
  std::shared_ptr<socket_notifier_operation>;

template<typename Func>
using function_socket_notifier_operation =
  parameterized_operation<Func, socket_t, notifier_type>;

template<class T>
using member_socket_notifier_operation =
  member_parameterized_operation<T, socket_t, notifier_type>;

/**
  Provides a notifier executing a lambda or an operation
  on a specific queue when the socket becomes readable or
  writable depending on the configured type.
  */
class XDISPATCH_EXPORT socket_notifier
{
public:
    /**
        Constructs a new notifier powered by the same backend as the target
       queue

        The notifier will be stopped, call resume() to execute it

        @param socket The socket to be monitored
        @param type The type of operation to monitor the socket for
        @param target The queue to execute the handler on, defaults to the
       global_queue
    */
    socket_notifier(socket_t socket,
                    notifier_type type,
                    const queue& target = global_queue());

    /**
        Constructs a new socket notifier using the given implementation

        @param impl The implementation to be used
        @param target The queue on which a handler is executed

        @throws std::logic_error if the impl backend is of a different type than
                the backend handling target
    */
    socket_notifier(const isocket_notifier_impl_ptr& impl, const queue& target);

    /**
        @brief Copy constructor
     */
    socket_notifier(const socket_notifier&) = default;

    /**
        @brief Move constructor
     */
    socket_notifier(socket_notifier&&) = default;

    /**
        @brief Destructor
     */
    ~socket_notifier() = default;

    /**
        Will resume the notifier.
        @remarks A new created notifier will be suspended and needs to be
       resumed first. Calls between resume() and suspend() need to be balanced.
    */
    void resume();

    /**
      Will suspend the notifier.
        @remarks A new created notifier will be suspended and needs to be
      resumed first. Calls between resume() and suspend() need to be balanced.
    */
    void suspend();

    /**
        @brief Cancels the notifier

        When the notifier has been cancelled no further handler invocations will
        be queued whenever the underlying socket becomes ready.

        A cancelled notifier cannot be reused again.

        This is safe to be invoked recursively, i.e. from within
        an active handler call in which case the current call will
        complete but no further calls be made.
     */
    void cancel();

    /**
        @brief assignment operator
     */
    socket_notifier& operator=(const socket_notifier&) = default;

    /**
        @brief move assignment operator
     */
    socket_notifier& operator=(socket_notifier&&) = default;

    /**
        Sets the operation to dispatch onto the target queue whenever
        the notifier becomes ready.
    */
    void handler(const socket_notifier_operation_ptr& op);

    /**
        Sets the operation to dispatch onto the target queue whenever
        the notifier becomes ready.
    */
    template<typename Func>
    inline void handler(const Func& f)
    {
        handler(socket_notifier_operation::make(f));
    }

    /**
        @returns the queue the handler will be executed on
    */
    queue target_queue() const;

    /**
        @returns the socket monitored by the notifier
    */
    socket_t socket() const;

    /**
        @returns the type of the notifier
    */
    notifier_type type() const;

private:
    isocket_notifier_impl_ptr m_impl;
    queue m_target_queue;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_SOCKET_NOTIFIER_H_ */
