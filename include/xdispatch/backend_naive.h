/*
 * backend_libdispatch.h
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

#ifndef XDISPATCH_BACKEND_NAIVE_H_
#define XDISPATCH_BACKEND_NAIVE_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"
#include "xdispatch/backend_naive_ithread.h"
#include "xdispatch/backend_naive_ithreadpool.h"
#if (!BUILD_XDISPATCH2_BACKEND_NAIVE)
    #error "The naive backend is not available on this platform"
#endif

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    @return A new serial queue powered by the given thread

    @param label The name to use for the new queue
    @param thread The thread on which all operations added to the queue
                  will be executed
    @param priority Choose a priority different from default to automatically
                  have the priority of the thread reconfigured
    */
XDISPATCH_EXPORT queue
create_serial_queue(const std::string& label,
                    const ithread_ptr& thread,
                    queue_priority priority = queue_priority::DEFAULT);

/**
    @return A new parallel queue powered by the given pool

    @param label The name to use for the new queue
    @param pool The threadpool on which queued operations will be executed
    @param priority Controls the priority assigned to draining the queue
                relative from other runnables added to the pool
    */
XDISPATCH_EXPORT queue
create_parallel_queue(const std::string& label,
                      const ithreadpool_ptr& pool,
                      queue_priority priority = queue_priority::DEFAULT);

} // namespace naive
__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_BACKEND_NAIVE_H_ */
