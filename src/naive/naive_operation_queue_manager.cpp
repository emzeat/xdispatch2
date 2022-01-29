/*
 * naive_operation_queue_manager.cpp
 *
 * Copyright (c) 2012 - 2022 Marius Zwicker
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

#include "naive_operation_queue_manager.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

operation_queue_manager::~operation_queue_manager()
{
    m_thread.cancel();
}

void
operation_queue_manager::attach(const void_ptr& q)
{
    m_thread.execute(make_operation([q, this] { m_queues.push_back(q); }));
}

void
operation_queue_manager::detach(const void* const q)
{
    m_thread.execute(make_operation([q, this] {
        for (auto it = m_queues.begin(); it != m_queues.end();) {
            if (it->get() == q) {
                it = m_queues.erase(it);
            } else {
                ++it;
            }
        }
    }));
}

operation_queue_manager&
operation_queue_manager::instance()
{
    // remark: intentionally leak this object to ensure it outlives any other
    // statics
    static auto* s_instance = new operation_queue_manager;
    return *s_instance;
}

operation_queue_manager::operation_queue_manager()
  : m_thread("de.mlba-team.xdispatch2.op_q_manager", queue_priority::BACKGROUND)
{}

} // namespace naive
__XDISPATCH_END_NAMESPACE
