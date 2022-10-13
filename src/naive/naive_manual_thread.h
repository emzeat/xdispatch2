/*
 * naive_manual_thread.h
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

#ifndef XDISPATCH_NAIVE_MANUAL_THREAD_H_
#define XDISPATCH_NAIVE_MANUAL_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    An implementation of ithreadpool which needs to be manually drained
 */
class manual_thread : public ithreadpool
{
public:
    /**
        @param name The name by which the thread is known
        @param priority The default priority for which the thread will execute
     */
    manual_thread(const std::string& name, queue_priority priority);

    /**
        @brief Destructor
     */
    ~manual_thread() override;

    /**
        Executes all queued operations on the calling thread
        until interruped via cancel()
     */
    void run();

    /**
        Signals an active call to drain() that
        it should return on the next possible occassion.
     */
    void cancel();

    /**
        @copydoc IThreadPool::execute
     */
    void execute(const operation_ptr& work,
                 queue_priority priority = queue_priority::DEFAULT) override;

    /**
        @copydoc IThreadPool::notify_thread_blocked
     */
    void notify_thread_blocked() override;

    /**
        @copydoc IThreadPool::notify_thread_unblocked
     */
    void notify_thread_unblocked() override;

private:
    const std::string m_name;
    const queue_priority m_priority;
    std::mutex m_CS;
    std::condition_variable m_cond;
    std::vector<operation_ptr> m_queued_ops;
    bool m_cancelled;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_MANUAL_THREAD_H_ */
