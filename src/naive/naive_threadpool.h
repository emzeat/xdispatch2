/*
 * naive_threadpool.h
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

#ifndef XDISPATCH_NAIVE_THREADPOOL_H_
#define XDISPATCH_NAIVE_THREADPOOL_H_

#include "naive_thread.h"
#include "naive_semaphore.h"
#include "naive_concurrentqueue.h"

#include <thread>
#include <atomic>
#include <array>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

using thread_ptr = std::shared_ptr<std::thread>;

/**
    An implementation of ithreadpool executing in a single thread
    for sake of simplicity.
 */
class threadpool : public ithreadpool
{
public:
    enum
    {
        bucket_USER_INTERACTIVE = 0,
        bucket_USER_INITIATED = 1,
        bucket_UTILITY = 2,
        bucket_BACKGROUND = 3,

        bucket_count
    };

    /**
        @brief Constructor
     */
    threadpool();

    /**
        @brief Destructor
     */
    ~threadpool() override;

    /**
        @copydoc ithreadpool::execute
     */
    void execute(const queued_operation& work, queue_priority priority) final;

    /**
        @brief Marks a thread as blocked, i.e. waiting on a resource

        Use this to notify the pool that it may spawn additional threads
        without overallocating the system's processor count as the calling
        thread is blocking on a resource
     */
    void notify_thread_blocked();

    /**
        @brief Marks a thread as unblocked, i.e. busy again

        Use this to notify the pool that a previously blocked thread
        has obtained its resource and will now make use of CPU resources
        again.
     */
    void notify_thread_unblocked();

    /**
        @return the shared pool instance
     */
    static std::shared_ptr<threadpool> instance();

private:
    class worker;
    class data;
    using data_ptr = std::shared_ptr<data>;

    void schedule();

    data_ptr m_data;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_THREADPOOL_H_ */
