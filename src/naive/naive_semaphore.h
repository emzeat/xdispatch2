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

#ifndef XDISPATCH_NAIVE_SEMAPHORE_H_
#define XDISPATCH_NAIVE_SEMAPHORE_H_

#include <condition_variable>
#include <mutex>
#include <atomic>

#include "naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

/**
    @brief An efficient implementation of a counting semaphore
 */
class semaphore
{
public:
    /**
        @brief Constructs a new semaphore with the initial count
     */
    explicit semaphore(
        int count = 0
    );

    /**
        @brief Tries to acquire the semaphore decrementing its
               count by one if the count is greater than zero

        @return true if the count was greater than zero before
                decrementing it, false if it was zero
     */
    bool try_acquire();

    /**
        @brief Tries to acquire the semaphore spinning in case
               it cannot be acquired directly

        @param spins The maximum number of attempts until giving up

        @return true if acquiring the semaphore succeeded or false if the
                maximum number of spins was reached first
     */
    bool spin_acquire(
        int spins
    );

    /**
        @brief Tries to acquire the semaphore blocking in case
               it cannot be acquired directly

        @param timeout The maximum time to wait attempting to acquire the semaphore

        @return true if acquiring the semaphore succeeded or false if the
                timeout was reached first
     */
    bool wait_acquire(
        std::chrono::milliseconds timeout
    );

    /**
        @brief Releases the semaphore incrementing its count

        @param count The increment by which to alter the count

        Any blocking calls to acquire() will be served in random order
        until the count is used up again or no blocking call is remaining.
     */
    void release(
        int count = 1
    );

private:
    std::atomic<int> m_count;
    std::mutex m_CS;
    std::condition_variable m_cond;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_SEMAPHORE_H_ */
