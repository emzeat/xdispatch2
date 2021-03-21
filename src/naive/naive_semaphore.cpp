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

#include <thread>

#include "naive_semaphore.h"
#include "../trace_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

semaphore::semaphore(int count)
  : m_count(count)
  , m_CS()
  , m_cond()
{}

bool
semaphore::try_acquire()
{
    auto old_count = m_count.load();
    do {
        XDISPATCH_ASSERT(old_count >= 0);
        if (0 == old_count) {
            return false;
        }
        if (m_count.compare_exchange_weak(old_count, old_count - 1)) {
            return true;
        }
    } while (true);
}

bool
semaphore::spin_acquire(int spins)
{
    for (int i = 0; i < spins; ++i) {
        if (try_acquire()) {
            return true;
        }

        std::this_thread::yield();
    }
    return false;
}

bool
semaphore::wait_acquire(std::chrono::milliseconds timeout)
{
    if (!try_acquire()) {
        std::unique_lock<std::mutex> lock(m_CS);
        // test again as we hold the lock this time
        if (!try_acquire()) {
            return m_cond.wait_for(
              lock, timeout, [this] { return try_acquire(); });
        }
    }
    return true;
}

void
semaphore::release(int count)
{
    XDISPATCH_ASSERT(count > 0);
    const auto previous = m_count.fetch_add(count);
    XDISPATCH_ASSERT(previous >= 0);

    // if drained before we need to notify
    if (0 == previous) {
        std::unique_lock<std::mutex> lock(m_CS);
        if (1 == count) {
            m_cond.notify_one();
        } else {
            m_cond.notify_all();
        }
    }
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
