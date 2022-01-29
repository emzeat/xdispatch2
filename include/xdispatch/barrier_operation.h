/*
 * barrier_operation.h
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
class XDISPATCH_EXPORT barrier_operation : public operation
{
public:
    barrier_operation();

    /**
        @brief Will wait for the operation to be executed

        If the operation was already executed,
        the call will return immediately.

        @param timeout the maximum time to wait for execution

        @returns true if the operation was executed or false if the timeout
                 expired before the operation completed its execution
     */
    bool wait(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(-1));

    /**
        @returns true if the operation was already executed.

        This is identical to calling wait( std::chrono::milliseconds( 0 ) )
        but implemented in a slightly more efficient manner
     */
    bool has_passed() const;

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    bool m_should_wait;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_BARRIER_OPERATION_H_
