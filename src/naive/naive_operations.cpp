/*
 * naive_operations.cpp
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

#include "naive_operations.h"
#include "naive_threadpool.h"

#include "xdispatch/impl/itimer_impl.h"
#include "../xdispatch_internal.h"

#include <thread>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

apply_operation::apply_operation(
  size_t index,
  const queued_parameterized_operation<size_t>& op,
  const consumable_ptr& consumable)
  : m_index(index)
  , m_op(op)
  , m_consumable(consumable)
{}

void
apply_operation::operator()()
{
    execute_operation_on_this_thread(m_op, m_index);

    if (m_consumable) {
        m_consumable->consume_resource();
    }
}

void
delayed_operation::create_and_dispatch(itimer_impl_ptr&& timer,
                                       std::chrono::milliseconds delay,
                                       const queued_operation& op)
{
    // this is using a little trick to make the operation self hosted
    // while still ensuring the timer object gets released accordingly:
    // 1) the operation is created and takes ownership of the timer
    // 2) the operation is assigned as handler to the timer. This creates
    //    a circular ownership which is accepted for the time being
    // 3) the timer is started with a delay
    // 4) the operation is executed by the timer at which point it will
    //    cancel and release the timer hence breaking the circular
    //    ownership and ensuring a clean destruction sequence

    auto delayed_op = std::make_shared<delayed_operation>(std::move(timer), op);
    auto queued_op = queue_operation_with_ctx(delayed_op, op.m_ctx);

    delayed_op->m_timer->handler(std::move(queued_op));
    delayed_op->m_timer->resume(delay);
}

delayed_operation::delayed_operation(itimer_impl_ptr&& timer,
                                     const queued_operation& op,
                                     const consumable_ptr& consumable)
  : m_timer(std::move(timer))
  , m_op(op)
  , m_consumable(consumable)
{}

void
delayed_operation::operator()()
{
    execute_operation_on_this_thread(m_op);

    if (m_consumable) {
        m_consumable->consume_resource();
    }

    if (m_timer) {
        m_timer->cancel();
        m_timer.reset();
    }
}

consuming_operation::consuming_operation(const queued_operation& op,
                                         const consumable_ptr& consumable)
  : m_op(op)
  , m_consumable(consumable)
{}

void
consuming_operation::operator()()
{
    execute_operation_on_this_thread(m_op);

    if (m_consumable) {
        m_consumable->consume_resource();
    }
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
