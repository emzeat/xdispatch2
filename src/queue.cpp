/*
 * queue.cpp
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

#include "xdispatch_internal.h"
#include "xdispatch/iqueue_impl.h"

__XDISPATCH_USE_NAMESPACE

queue::queue(const std::string& label, const iqueue_impl_ptr& impl)
  : m_impl(impl)
  , m_label(label)
{
    XDISPATCH_ASSERT(m_impl);
}

void
queue::async(const operation_ptr& op) const
{
    XDISPATCH_ASSERT(op);
    queue_operation_with_d(*op, m_impl.get());
    m_impl->async(op);
}

void
queue::apply(size_t times, const iteration_operation_ptr& op) const
{
    XDISPATCH_ASSERT(op);
    queue_operation_with_d(*op, m_impl.get());
    m_impl->apply(times, op);
}

void
queue::after(std::chrono::milliseconds delay, const operation_ptr& op) const
{
    XDISPATCH_ASSERT(op);
    m_impl->after(delay, op);
}

std::string
queue::label() const
{
    return m_label;
}

bool
queue::operator==(const queue& other) const
{
    return m_impl == other.m_impl;
}

iqueue_impl_ptr
queue::implementation() const
{
    return m_impl;
}

bool
queue::is_current_queue() const
{
    return operation_is_run_with_d(m_impl.get());
}
