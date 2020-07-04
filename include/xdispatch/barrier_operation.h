/*
* barrier_operation.h
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

#ifndef XDISPATCH_BARRIER_OPERATION_H_
#define XDISPATCH_BARRIER_OPERATION_H_

#include "xdispatch/operation.h"

#include <mutex>
#include <condition_variable>

__XDISPATCH_BEGIN_NAMESPACE

/**
 * @brief Implements a barrier operation which may be used to synchronize
 *        with the execution of an operation
 */
class barrier_operation : public operation
{
public:
    barrier_operation();

    /**
        @brief Will wait for the operation to be executed

        @param timeout the maximum time to wait for execution

        @returns true if the operation was executed or false if the timout
                 expired before the operation completed its execution
     */
    bool wait(
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    );

    void operator()() final;

private:
    bool m_should_wait;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

__XDISPATCH_END_NAMESPACE


#endif // XDISPATCH_BARRIER_OPERATION_H_
