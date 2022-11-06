/*
 * naive_semaphore.cpp
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

#include <thread>

#include "naive_semaphore.h"
#include "../thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

semaphore::semaphore(int count)
  : m_count(count)
  , m_waiters(0)
  , m_CS()
  , m_cond()
{
    XDISPATCH_ASSERT(m_count.is_lock_free());
    XDISPATCH_ASSERT(m_waiters.is_lock_free());
}

bool
semaphore::try_acquire()
{
    // use consume semantics since we do not require a full
    // write barrier here but only depend on any changes to
    // m_count to be visible. The full barrier will be issued
    // by compare_exchange_weak below instead and would only
    // be duplicated if done up here as well.
    auto old_count = m_count.load(std::memory_order_consume);
    do {
        XDISPATCH_ASSERT(old_count >= 0);
        if (0 == old_count) {
            return false;
        }
        if (m_count.compare_exchange_weak(
              old_count, old_count - 1, std::memory_order_seq_cst)) {
            return true;
        }
        // compare exchange failed, old_count was updated with the actual value
    } while (true);
}

bool
semaphore::spin_acquire(int spins)
{
    for (int i = 0; i < spins; ++i) {
        if (try_acquire()) {
            return true;
        }

        // note discussions linked at
        // https://en.cppreference.com/w/cpp/atomic/atomic_flag most notably
        // https://www.realworldtech.com/forum/?threadid=189711&curpostid=189723
        // which comes to the conclusion that doing sched_yield may be far from
        // ideal on SMP systems and actually be worse than suspending the thread
        // on a mutex especially considering this involves a syscall.
        //
        // So instead try to follow
        // https://www.realworldtech.com/forum/?threadid=189711&curpostid=189755
        // which basically suggests to do a plain read + relax operation instead
        // to avoid stumping on cache lines while just waiting on the counter to
        // be increased.
        while (0 == m_count.load(std::memory_order_consume) && i < spins) {
            thread_utils::cpu_relax();
            ++i;
        }
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

            struct waiter_scope
            {
                explicit waiter_scope(std::atomic<int>& waiters)
                  : m_waiters(waiters)
                {
                    m_waiters.fetch_add(1, std::memory_order_release);
                }
                waiter_scope(const waiter_scope& other) = delete;
                ~waiter_scope()
                {
                    m_waiters.fetch_sub(1, std::memory_order_release);
                }

            private:
                std::atomic<int>& m_waiters;
            };

            waiter_scope waiting(m_waiters);
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
    m_count.fetch_add(count, std::memory_order_seq_cst);

    // if there is waiters we should notify them
    if (0 != m_waiters.load(std::memory_order_acquire)) {
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
