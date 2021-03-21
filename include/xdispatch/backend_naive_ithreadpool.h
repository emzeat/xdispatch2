/*
 * ithreadpool.h
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

#ifndef XDISPATCH_NAIVE_ITHREADPOOL_H_
#define XDISPATCH_NAIVE_ITHREADPOOL_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    @brief Defines an interface to be implemented by a thread pool instance

    The pool will be notified whenever work is pending that should be
    executed.
 */
class ithreadpool
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
};

using ithreadpool_ptr = std::shared_ptr<ithreadpool>;

} // namespace naive
__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_NAIVE_ITHREADPOOL_H_ */
