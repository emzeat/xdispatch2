/*
 * cancelable.cpp
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

#include "xdispatch/impl/cancelable.h"
#include "xdispatch_internal.h"

#include <cstdlib>
#include <set>

__XDISPATCH_BEGIN_NAMESPACE

static thread_local std::set<cancelable*> current_c;

cancelable::cancelable()
  : m_active(active_enabled)
{}

void
cancelable::disable()
{
    if (current_c.find(this) != current_c.end()) {
        // recursion
        m_active.store(active_disabled);
    } else {
        auto current = m_active.exchange(active_disabled);
        if (active_running == current) {
            // tried to disable while still running
            m_barrier.wait();
        }
    }
}

bool
cancelable::enter()
{
    auto expected = active_enabled;
    if (m_active.compare_exchange_strong(expected, active_running)) {
        current_c.insert(this);
        return true;
    }
    // disabled
    return false;
}

void
cancelable::leave()
{
    current_c.erase(this);
    auto expected = active_running;
    if (!m_active.compare_exchange_strong(expected, active_enabled)) {
        // disabled in the meantime
        m_barrier.complete();
    }
}

__XDISPATCH_END_NAMESPACE
