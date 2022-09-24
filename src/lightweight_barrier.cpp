/*
 * lightweight_barrier.cpp
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

#include "xdispatch/impl/lightweight_barrier.h"
#include "xdispatch_internal.h"

#include <cstdlib>

__XDISPATCH_BEGIN_NAMESPACE

// Indicates the barrier has nobody waiting and has not been completed
static constexpr lightweight_barrier::waiter* kNoOwner{ nullptr };

// Indicates the barrier has been completed
static char kInvalidAddressUsedForCompleted = 0;
static lightweight_barrier::waiter* const kCompleted{
    reinterpret_cast<lightweight_barrier::waiter*>(
      &kInvalidAddressUsedForCompleted)
};

class lightweight_barrier::waiter
{
public:
    waiter()
      : m_done(false)
    {}

    inline bool wait(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(-1))
    {
        auto predicate = [this] { return m_done; };

        std::unique_lock<std::mutex> lock(m_mutex);
        if (!predicate()) {
            if (std::chrono::milliseconds(-1) != timeout) {
                return m_cond.wait_for(lock, timeout, predicate);
            }
            m_cond.wait(lock, predicate);
            return true;
        }
        return true;
    }

    inline void complete()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_done = true;
        m_cond.notify_all();
    }

    inline bool was_completed()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_done;
    }

private:
    bool m_done;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

lightweight_barrier::lightweight_barrier()
  : m_owner{ kNoOwner }
{}

lightweight_barrier::~lightweight_barrier()
{
    auto* owner = m_owner.load(std::memory_order_acquire);
    if (owner && owner != kCompleted) {
        delete owner;
    }
}

bool
lightweight_barrier::wait(std::chrono::milliseconds timeout)
{
    // avoid allocating a waiter object unecessarily
    // by checking the atomic value first to see if we
    // can bail out quickly
    waiter* previous = m_owner.load(std::memory_order_acquire);
    if (kCompleted == previous) {
        return true;
    }

    // try to become the owner of the mutex
    auto candidate = std::make_unique<waiter>();
    previous = kNoOwner;
    if (m_owner.compare_exchange_weak(
          previous, candidate.get(), std::memory_order_acq_rel)) {
        // value was incomplete before, we are the owner now so wait
        auto* barrier = candidate.release();
        return barrier->wait(timeout);
    }
    if (kCompleted == previous) {
        // value was (and still is) complete, so no need to block
        return true;
    }
    // else: somebody else placed a waiter barrier before, use it instead
    // (previous holds the barrier obtained from compare_exchange_weak above)
    return previous->wait(timeout);
}

void
lightweight_barrier::complete()
{
    auto* previous = kNoOwner;
    if (m_owner.compare_exchange_weak(
          previous, kCompleted, std::memory_order_acq_rel)) {
        // nobody was waiting and now marked as complete
    } else if (kCompleted == previous) {
        // complete before
    } else if (kNoOwner != previous) {
        // somebody is waiting
        previous->complete();
    }
}

bool
lightweight_barrier::was_completed() const
{
    waiter* previous = m_owner.load(std::memory_order_acquire);
    if (kCompleted == previous) {
        return true;
    }
    if (kNoOwner != previous) {
        return previous->was_completed();
    }
    return false;
}

__XDISPATCH_END_NAMESPACE
