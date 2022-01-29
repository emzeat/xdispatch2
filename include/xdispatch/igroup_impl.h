/*
 * igroup_impl.h
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

#ifndef XDISPATCH_IGROUP_IMPL_H_
#define XDISPATCH_IGROUP_IMPL_H_

/**
 * @addtoigroup_impl xdispatch
 * @{
 */

#include "xdispatch/ibackend.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
    @brief interface to be implemented to support groups
*/

class igroup_impl
{
public:
    /**
        @brief Destructor
     */
    virtual ~igroup_impl() = default;

    /**
        Dispatches an operation on the given Queue
        @param op The operation to be dispatched
        @param q The Queue to use. If no Queue is given, the system default
       queue will be used
    */
    virtual void async(const operation_ptr& op, const iqueue_impl_ptr& q) = 0;

    /**
        Waits until the given time has passed
        or all dispatched operations in the igroup_impl were executed

        @param timeout give a time here or a DISPATCH_TIME_FOREVER to wait until
       all operations are done

        @return false if the timeout occured or true if all operations were
       executed
    */
    virtual bool wait(std::chrono::milliseconds timeout) = 0;

    /**
        This function schedules a notification operation to be submitted to the
       specified queue once all operations associated with the dispatch
       igroup_impl have completed.

        If no operations are associated with the dispatch igroup_impl (i.e. the
       igroup_impl is empty) then the notification operation will be submitted
       immediately.

        The operation will be empty at the time the notification block is
       submitted to the target queue immediately. The igroup_impl may either be
       deleted or reused for additional operations.
    */
    virtual void notify(const operation_ptr& op, const iqueue_impl_ptr& q) = 0;

    /**
        @returns the backend type behind this implementation
     */
    virtual backend_type backend() = 0;

protected:
    igroup_impl() = default;

private:
    igroup_impl(const igroup_impl&) = delete;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_IGROUP_IMPL_H_ */
