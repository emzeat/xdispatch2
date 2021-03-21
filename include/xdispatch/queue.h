/*
 * queue.h
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

#ifndef XDISPATCH_QUEUE_H_
#define XDISPATCH_QUEUE_H_

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

class iqueue_impl;
using iqueue_impl_ptr = std::shared_ptr<iqueue_impl>;

/**
    Provides an interface for representing
    a dispatch queue and methods that can be
    called to modify or use the queue.

    Read Apple's documentation of libDispatch
    to understand the concept of tasks and
    queues.

    @see xdispatch::dispatch for creation of queues
*/
class XDISPATCH_EXPORT queue
{
public:
    /**
        @brief Creates a new queue using the platform default backend and label.
     */
    explicit queue(const std::string& label,
                   queue_priority priority = queue_priority::DEFAULT);

    /**
        @brief Creates a new queue using the given implementation and label.
     */
    queue(const std::string& label, const iqueue_impl_ptr& impl);

    /**
        @brief copy constructor
     */
    queue(const queue&) = default;

    /**
        @brief move constructor
     */
    queue(queue&&) = default;

    /**
        @brief destructor
    */
    ~queue() = default;

    /**
        Will dispatch the given operation for async execution on the queue and
       return immediately.

        The queue will be retained by the system until the operation was
       executed.
      */
    void async(const operation_ptr& op) const;

    /**
        @see async(operation_ptr).

        Will put the given function on the queue.
        The group and queue will be retained by the system until the operation
       was executed.
    */
    template<typename Func>
    inline void async(const Func& f) const
    {
        async(make_operation(f));
    }

    /**
        Applies the given iteration_operation for async execution
        in this queue and waits for all iterations of the operation to complete
       execution.

        @param times The number of times the operation will be executed
    */
    void apply(size_t times, const iteration_operation_ptr& op) const;

    /**
        @see apply(sizee_t, iteration_operation_ptr).

        Will wrap the given function in an operation and put it on the queue.
    */
    template<typename Func>
    inline void apply(size_t times, const Func& f) const
    {
        apply(times, make_iteration_operation(f));
    }

    /**
        Applies the given operation for async execution
        in this queue after the given time and returns immediately.

        The queue will be retained by the system until the operation was
       executed.

        @param delay The time to wait until the operation is applied to
                     the queue.
    */
    void after(std::chrono::milliseconds delay, const operation_ptr& op) const;

    /**
        @see after(std::chrono::milliseconds, operation_ptr)

        Will wrap the given function in an operation and put it on the
        queue for execution as soon as the delay expired.
    */
    template<typename Func>
    inline void after(std::chrono::milliseconds delay, const Func& f) const
    {
        after(delay, make_operation(f));
    }

    /**
        @return The label of the queue that was used while creating it
    */
    std::string label() const;

    /**
        @brief Assignment operator
    */
    queue& operator=(const queue&) = default;

    /**
        @brief Equality operator
    */
    bool operator==(const queue& other) const;

    /**
        @brief Unequality operator
    */
    inline bool operator!=(const queue& other) const
    {
        return !(*this == other);
    }

    /**
        @return The implementation instance backing this queue
     */
    iqueue_impl_ptr implementation() const;

    /**
        @brief helper to test if the queue is active on this thread

        Active means that an operation added to this queue is currently
       executing on the thread invoking this function. This can be used to
       detect recursion.
     */
    bool is_current_queue() const;

private:
    iqueue_impl_ptr m_impl;
    std::string m_label;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_QUEUE_H_ */
