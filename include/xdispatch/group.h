/*
 * group.h
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

#ifndef XDISPATCH_GROUP_H_
#define XDISPATCH_GROUP_H_

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

class igroup_impl;
using igroup_impl_ptr = std::shared_ptr<igroup_impl>;

/**
    A group is a group of operations
    dispatched on queues. This class provides
    a way to track all these operations
    and notify you if all of them finished
    executing.

    See also Apple's Documentation of Dispatch Groups
*/

class XDISPATCH_EXPORT group
{
public:
    /**
        @brief Creates a new group powered by the platform default backend
        */
    group();

    /**
        @brief Creates a new group using the given impl
     */
    explicit group(const igroup_impl_ptr&);

    /**
        @brief copy constructor
     */
    group(const group&) = default;

    /**
        @brief move constructor
     */
    group(group&&) = default;

    /**
        @brief Destructor
     */
    ~group() = default;

    /**
        Dispatches an operation on the given queue
        @param op The operation to be dispatched
        @param q The queue to use. If no queue is given, the system default
       queue will be used

        The group and queue will be retained by the system until the operation
       was executed.
    */
    void async(const operation_ptr& op, const queue& q = global_queue()) const;

    /**
        Dispatches an operation on the global queue with the given priority
        @param op The operation to be dispatched
        @param priority The priority of the global queue to be used

        The group and queue will be retained by the system until the operation
       was executed.
    */
    void async(const operation_ptr& op, queue_priority priority) const;

    /**
        @see async(operation_ptr, queue)

        Will wrap the given function in an operation and put it on the queue.
        The group and queue will be retained by the system until the operation
       was executed.
    */
    template<typename Func>
    inline void async(const Func& f, const queue& q = global_queue()) const
    {
        async(make_operation(f), q);
    }

    /**
        @see async(operation_ptr, queue)

        Will wrap the given function in an operation and put it on the queue
       with the given priority The group and queue will be retained by the
       system until the operation was executed.
    */
    template<typename Func>
    inline void async(const Func& f, queue_priority priority) const
    {
        async(make_operation(f), priority);
    }

    /**
        This function schedules a notification operation to be submitted to the
       specified queue once all operations associated with the dispatch group
       have completed.

        If no operations are associated with the dispatch group (i.e. the group
       is empty) then the notification operation will be submitted immediately.

        The operation will be empty at the time the notification block is
       submitted to the target queue immediately. The group may either be
       deleted or reused for additional operations.
    */
    void notify(const operation_ptr& op, const queue& q = global_queue()) const;

    /**
        This function schedules a notification function to be submitted to the
       specified queue once all operations associated with the dispatch group
       have completed.

        If no blocks are associated with the dispatch group (i.e. the group is
       empty) then the notification function will be submitted immediately.

        The group will be empty at the time the notification function is
       submitted to the target queue. The group may either be deleted or reused
       for additional operations.
    */
    template<typename Func>
    inline void notify(const Func& f, const queue& q = global_queue()) const
    {
        notify(make_operation(f), q);
    }

    /**
        Waits until the given time has passed
        or all dispatched operations in the group were executed

        @param t give a time here, will wait forever by default
        @return false if the timeout occured or true if all operations were
       executed
    */
    bool wait(
      std::chrono::milliseconds t = std::chrono::milliseconds(-1)) const;

    /**
        @brief assignment operator
     */
    group& operator=(const group&) = default;

    /**
        @brief move assignment operator
     */
    group& operator=(group&&) = default;

private:
    igroup_impl_ptr m_impl;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_GROUP_H_ */
