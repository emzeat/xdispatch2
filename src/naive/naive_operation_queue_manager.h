/*
 * naive_operation_queue_manager.h
 *
 * Copyright (c) 2012 - 2023 Marius Zwicker
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

#ifndef XDISPATCH_NAIVE_OPERATION_QUEUE_MANAGER_H_
#define XDISPATCH_NAIVE_OPERATION_QUEUE_MANAGER_H_

#include <vector>

#include "naive_backend_internal.h"
#include "naive_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

using void_ptr = std::shared_ptr<void>;

/**
    @brief Provides the root for all operation queues

    This is used by operation queues to manage their lifetime
    such that an operation queue will only go out of scope when
    its last operation goes out of scope.

    The manager is optimized
 */
class operation_queue_manager
{
public:
    ~operation_queue_manager();

    /**
        @brief Attaches the given queue to the manager

        Once the call returns the queue will remain alive until
        explicitly detached from the manager again.

        It is safe to call this function from multiple
        threads at the same time.
     */
    void attach(const void_ptr& q);

    /**
        @brief Detaches the given queue from the manager

        Once the call returns the queue will go out of
        any time - there is no guarantee as to when.

        It is safe to call this function from multiple
        threads at the same time.
     */
    void detach(void const* q);

    /**
        @return The global instance of the operation manager
     */
    static operation_queue_manager& instance();

private:
    operation_queue_manager();

    thread m_thread;
    std::vector<void_ptr> m_queues;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_NAIVE_OPERATION_QUEUE_MANAGER_H_
