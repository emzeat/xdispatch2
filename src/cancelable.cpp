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

#include "xdispatch/cancelable.h"

#include <cstdlib>

__XDISPATCH_BEGIN_NAMESPACE

cancelable::cancelable()
  : m_active(active_enabled)
{}

void
cancelable::disable(const queue& executor_queue)
{
    if (executor_queue.is_current_queue()) {
        // recursion
        m_active.store(active_disabled);
    } else {
        auto expected = active_enabled;
        do {
            expected = active_enabled;
            m_active.compare_exchange_strong(expected, active_disabled);
        } while (active_running == expected);
    }
}

void
cancelable::enable()
{
    m_active.store(active_enabled);
}

bool
cancelable::enter()
{
    auto expected = active_enabled;
    return m_active.compare_exchange_strong(expected, active_running);
}

void
cancelable::leave()
{
    auto expected = active_running;
    m_active.compare_exchange_strong(expected, active_enabled);
}

__XDISPATCH_END_NAMESPACE
