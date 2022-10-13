/*
 * naive_manual_thread.cpp
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

#include "naive_manual_thread.h"
#include "../thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

manual_thread::manual_thread(const std::string& name, queue_priority priority)
  : ithreadpool()
  , m_name(name)
  , m_priority(priority)
  , m_cancelled(false)
{}

manual_thread::~manual_thread() = default;

void
manual_thread::execute(const operation_ptr& work, queue_priority /* priority */
)
{
    std::lock_guard<std::mutex> guard(m_CS);
    m_queued_ops.push_back(work);
    m_cond.notify_all();
}

void
manual_thread::notify_thread_blocked()
{}

void
manual_thread::notify_thread_unblocked()
{}

void
manual_thread::run()
{
    thread_utils::set_current_thread_name(m_name);
    thread_utils::set_current_thread_priority(m_priority);
    while (!m_cancelled) {
        std::vector<operation_ptr> active_ops;
        {
            std::unique_lock<std::mutex> guard(m_CS);
            if (m_queued_ops.empty()) {
                m_cond.wait(guard);
            }
            std::swap(m_queued_ops, active_ops);
        }

        for (const auto& op : active_ops) {
            execute_operation_on_this_thread(*op);
        }
    }

    m_cancelled = false;
}

void
manual_thread::cancel()
{
    std::lock_guard<std::mutex> guard(m_CS);
    m_cancelled = true;
    m_cond.notify_all();
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
