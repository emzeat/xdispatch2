/*
 * backend_naive_ithreadpool.h
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
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

#ifndef XDISPATCH_NAIVE_ITHREADPOOL_H_
#define XDISPATCH_NAIVE_ITHREADPOOL_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class ithreadpool;
using ithreadpool_ptr = std::shared_ptr<ithreadpool>;

/**
    @brief Defines an interface to be implemented by a thread pool instance

    The pool will be notified whenever work is pending that should be
    executed.
 */
class XDISPATCH_EXPORT ithreadpool
{
public:
    ithreadpool() = default;
    ithreadpool(const ithreadpool& other) = delete;
    virtual ~ithreadpool() = default;

    /**
        @brief Notifies the pool about the new work to be executed

        @param work the work to be executed on the thread
        @param priority The priority with which the work is to be executed

        Notify may be invoked from multiple threads at the same time and also
       while a previously scheduled work is actively executing.
     */
    virtual void execute(const operation_ptr& work,
                         queue_priority priority) = 0;

    /**
        @brief Returns the threadpool instance currently executing this thread
       or null
     */
    static ithreadpool* current();

    /**
        @brief Helper to mark a thread as blocked, i.e. not running anymore.

        Will automatically notify the underlying threadpool (if any) so that
        thread counts can be adapted accordingly
     */
    class block_scope
    {
    public:
        block_scope();
        block_scope(ithreadpool*);
        block_scope(const block_scope&) = delete;
        ~block_scope();

    private:
        ithreadpool* m_pool;
    };

protected:
    /**
        @brief Marks a thread as blocked, i.e. waiting on a resource

        Use this to notify the pool that it may spawn additional threads
        without overallocating the system's processor count as the calling
        thread is blocking on a resource
     */
    virtual void notify_thread_blocked() = 0;

    /**
        @brief Marks a thread as unblocked, i.e. busy again

        Use this to notify the pool that a previously blocked thread
        has obtained its resource and will now make use of CPU resources
        again.
     */
    virtual void notify_thread_unblocked() = 0;

    /**
        @brief Runs the given operation in the scope of the given threadpool
    */
    static void run_with_threadpool(operation&, ithreadpool*);
};

inline ithreadpool::block_scope::block_scope()
  : block_scope(ithreadpool::current())
{}

inline ithreadpool::block_scope::block_scope(ithreadpool* pool)
  : m_pool(pool)
{
    if (m_pool) {
        m_pool->notify_thread_blocked();
    }
}

inline ithreadpool::block_scope::~block_scope()
{
    if (m_pool) {
        m_pool->notify_thread_unblocked();
    }
}

} // namespace naive
__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_NAIVE_ITHREADPOOL_H_ */
