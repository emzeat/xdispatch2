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
    An efficient implementation of a counter acting like a semaphore
 */
class semaphore
{
public:
    explicit semaphore(
        int count = 0
    );

    bool try_acquire();

    void acquire();

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
