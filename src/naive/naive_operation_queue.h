/*
 * naive_operation_queue.h
 *
 * Copyright (c) 2012 - 2022 Marius Zwicker
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

#ifndef XDISPATCH_NAIVE_CONTEXTQUEUE_H_
#define XDISPATCH_NAIVE_CONTEXTQUEUE_H_

#include <list>
#include <mutex>

#include "naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    A threadsafe queue which will maintain operations for execution

    After creation the owner of the operation_queue is required to
    attach() it so that the queue registers with the manager and its
    notification and lifetime management become active.

    If an operation is queued it will be automatically dispatched
    onto the associated thread. Unnecessary thread wakeups will
    be optimized by not waking an already active thread again.

    As soon as the owner has no use for the operation_queue
    and also has no intend to queue operations to it anymore, it
    is required to detach() it. This will dispatch one last
    operation which acts as a kind of barrier making sure that
    the queue unregisters with the manager again and goes out of
    scope with the last operation completing.

    @see operation_queue_manager
 */
class operation_queue : public std::enable_shared_from_this<operation_queue>
{
public:
    /**
        @param threadpool The threadpool implementation that all queued
       operations will be eventually executed on
        @param label The label by which the queue is known
        @param priority The priority at which the queue operates
     */
    operation_queue(const ithreadpool_ptr& thread,
                    const std::string& label,
                    queue_priority priority);

    /**
        @brief Destructor
     */
    ~operation_queue();

    /**
        @brief Enqueues the passed job for async execution in the queue

        @remark A queue needs to have been attached for this to show an effect
     */
    void async(const queued_operation& job);

    /**
        @brief Marks the queue as active

        After attaching a queue it is registered with the queue manager
        and may receive operations for async execution. It is ensured
        that the queue object cannot go out of scope until detach()
        has been called.

        @see operation_queue_manager
     */
    void attach();

    /**
        @brief Marks the queue as inactive

        No further operations can be queued for async execution anymore.

        The queue will unregister from the queue manager as soon
        as the last operation which had been queued until this point
        has executed. After unregistering the queue will go out of scope.

        @see operation_queue_manager
     */
    void detach();

private:
    const std::string m_label;
    const queue_priority m_priority;
    std::list<queued_operation> m_jobs;
    std::mutex m_CS;
    bool m_active_drain;
    queued_operation m_notify_operation;
    ithreadpool_ptr m_threadpool;

    void drain();

    static void process_job(const queued_operation& job);
};

using operation_queue_ptr = std::shared_ptr<operation_queue>;

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_NAIVE_CONTEXTQUEUE_H_
