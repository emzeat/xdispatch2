/*
 * timer.cpp
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
#include "xdispatch/impl/itimer_impl.h"
#include "xdispatch/impl/iqueue_impl.h"
#include "trace_utils.h"

__XDISPATCH_USE_NAMESPACE

timer::timer(const itimer_impl_ptr& impl, const queue& target)
  : m_impl(impl)
  , m_target_queue(target)
{
    XDISPATCH_ASSERT(m_impl);

    const auto target_impl = target.implementation();
    trace_utils::assert_same_backend(m_impl->backend(), target_impl->backend());
}

void
timer::interval(std::chrono::milliseconds interval)
{
    m_impl->interval(interval);
}

void
timer::latency(timer_precision precision)
{
    m_impl->latency(precision);
}

void
timer::resume(std::chrono::milliseconds d)
{
    m_impl->resume(d);
}

void
timer::handler(const operation_ptr& op)
{
    queue_operation_with_d(*op, m_target_queue.implementation().get());
    m_impl->handler(op);
}

queue
timer::target_queue() const
{
    return m_target_queue;
}
