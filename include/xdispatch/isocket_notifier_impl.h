/*
 * isocket_notifier_impl.h
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

#ifndef XDISPATCH_ISOCKET_NOTIFIER_IMPL_H_
#define XDISPATCH_ISOCKET_NOTIFIER_IMPL_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/ibackend.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
  @brief interface to be implemented to support a socket notifier
  */
class isocket_notifier_impl
{
public:
    /**
        @brief Destructor
    */
    virtual ~isocket_notifier_impl() = default;

    /**
        Sets the operation to dispatch onto the target queue whenever
        the notifier becomes ready.
    */
    virtual void handler(const socket_notifier_operation_ptr& op) = 0;

    /**
        Sets the queue the handler will be executed on
    */
    virtual void target_queue(const iqueue_impl_ptr&) = 0;

    /**
        Start the notifier
      */
    virtual void start() = 0;

    /**
        Will stop the timer.
    */
    virtual void stop() = 0;

    /**
        @returns the socket monitored by the notifier
    */
    virtual socket_t socket() const = 0;

    /**
        @returns the type of the notifier
    */
    virtual notifier_type type() const = 0;

    /**
        @returns the backend type behind this implementation
     */
    virtual backend_type backend() = 0;

protected:
    isocket_notifier_impl() = default;

private:
    isocket_notifier_impl(const isocket_notifier_impl&) = delete;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_ISOCKET_NOTIFIER_IMPL_H_ */
