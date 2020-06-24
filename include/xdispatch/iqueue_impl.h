/*
* iqueue_impl.h
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


#ifndef XDISPATCH_IQUEUE_IMPL_H_
#define XDISPATCH_IQUEUE_IMPL_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
    @brief interface to be implemented to support a queue
*/
class iqueue_impl
{
public:
    /**
        @brief destructor
    */
    virtual ~iqueue_impl() = default;

    /**
      Will dispatch the given operation for
      async execution on the iqueue_impl and return
      immediately.

      The operation will be deleted as soon
      as it was executed. To change this behaviour,
      set the auto_delete flag of the operation.
      @see operation::auto_delete()
      */
    virtual void async(
        const operation_ptr& op
    ) = 0;

    /**
        Applies the given iteration_operation for async execution
        in this iqueue_impl and returns immediately.

        @param times The number of times the operation will be executed
    */
    virtual void apply(
        size_t times,
        const iteration_operation_ptr& op
    ) = 0;

    /**
        Applies the given operation for async execution
        in this iqueue_impl after the given time and returns immediately.

        @param delay The time to wait until the operation is applied to
                     the iqueue_impl.
    */
    virtual void after(
        std::chrono::milliseconds delay,
        const operation_ptr& op
    ) = 0;

    /**
        @returns the backend type behind this implementation
     */
    virtual backend_type backend() = 0;

private:
    iqueue_impl(
        const iqueue_impl&
    ) = delete;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_IQUEUE_IMPL_H_ */
