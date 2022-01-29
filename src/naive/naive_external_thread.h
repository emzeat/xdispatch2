/*
 * naive_external_thread.h
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

#ifndef XDISPATCH_NAIVE_EXTERNAL_THREAD_H_
#define XDISPATCH_NAIVE_EXTERNAL_THREAD_H_

#include "xdispatch/backend_naive_ithread.h"
#include "xdispatch/backend_naive_ithreadpool.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    An implementation of a threadpool executed on an externally provided thread
 */
class external_thread : public ithreadpool
{
public:
    /**
        @param thread The thread on which the pool will be executed
     */
    explicit external_thread(const ithread_ptr& thread);

    /**
        @brief Destructor
     */
    ~external_thread() override;

    /**
        @copydoc ithreadpool::execute
     */
    void execute(const operation_ptr& work, queue_priority priority) final;

private:
    ithread_ptr m_thread;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_EXTERNAL_THREAD_H_ */
