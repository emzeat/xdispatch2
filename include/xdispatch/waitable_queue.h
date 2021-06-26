/*
 * waitable_queue.h
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

#ifndef XDISPATCH_WAITABLE_QUEUE_H_
#define XDISPATCH_WAITABLE_QUEUE_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
    Provides a wrapper around any queue which can be helpful in case
    synchronization is required with operations added to the queue.

    The difference to issueing a barrier_operation is that this will ensure that
    the wait can never deadlock by executing operations on the blocking thread
    in case no other thread is actively executing an operation of the queue.

    This is preferrable over a barrier_operation as it cannot deadlock even on
    systems heavily competing for CPU resources.

    However please note that this is still adding overhead compared to using a
    queue directly and as such should only be used when truly needed.
*/
class XDISPATCH_EXPORT waitable_queue : public queue
{
public:
    /**
        @brief Creates a new waitable queue using a private queue for delegation

        The queue will be created using the platform default backend

        @param label The name to be given to the private queue
        @param priority The priority to assign to the new queue
     */
    explicit waitable_queue(const std::string& label,
                            queue_priority priority = queue_priority::DEFAULT);

    /**
        @brief Creates a new waitable_queue delegating to the provided queue

        @param label The label to assign to this queue
        @param inner_queue The queue to be used for async execution

        @remark Operations queued to this waitable_queue and directly queued to
        the inner_queue used for delegation may sill be executed in parallel.
     */
    explicit waitable_queue(const std::string& label, const queue& inner_queue);

    /**
        @brief Waits for a previously queued operation to complete

        Will return immediately if no operation has been queued at all
        or if an operation has been completed before which had not been waited
        for.

        Will block the caller in case no operation has been completed and
        wait for at least one such operation to complete.
    */
    void wait_for_one();

    /**
        @brief Similar to wait_for_one() but will block for all operations
               queued at the time this is invoked to be completed
    */
    void wait_for_all();

private:
    class impl;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_WAITABLE_QUEUE_H_ */
