/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/

#ifndef XDISPATCH_NAIVE_THREADPOOL_H_
#define XDISPATCH_NAIVE_THREADPOOL_H_

#include "naive_thread.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

using thread_ptr = std::shared_ptr< std::thread >;

/**
    An implementation of ithreadpool executing in a single thread
    for sake of simplicity.
 */
class threadpool : public ithreadpool
{
public:
    /**
        @brief Constructor
     */
    threadpool();

    /**
        @brief Destructor
     */
    ~threadpool() final;

    /**
        @copydoc ithreadpool::execute
     */
    void execute(
        const operation_ptr& work,
        const queue_priority priority
    ) final;

    /**
        @brief Marks a thread as blocked, i.e. waiting on a resource

        Use this to notify the pool that it may spawn additional threads
        without overallocating the system's processor count as the calling
        thread is blocking on a resource
     */
    void thread_blocked();

    /**
        @brief Marks a thread as unblocked, i.e. busy again

        Use this to notify the pool that a previously blocked thread
        has obtained its resource and will now make use of CPU resources
        again.
     */
    void thread_unblocked();

    /**
        @return the shared pool instance
     */
    static ithreadpool_ptr instance();

private:
    void schedule();
    void run_thread();

    std::mutex m_CS;
    std::condition_variable m_cond;
    std::size_t m_max_threads;
    std::vector< thread_ptr > m_threads;
    std::size_t m_idle_threads;
    std::list< operation_ptr > m_operations;
    bool m_cancelled;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_THREADPOOL_H_ */
