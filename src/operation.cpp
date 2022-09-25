/*
 * operation.cpp
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

__XDISPATCH_BEGIN_NAMESPACE

static thread_local void* current_target = nullptr;

queued_operation
queue_operation_with_target(const operation_ptr& op, void* target)
{
    queued_operation queued;
    queued.m_ctx.m_target = target;
    queued.m_op = op;
    return queued;
}

queued_operation
queue_operation_with_ctx(const operation_ptr& op, const queued_ctx& ctx)
{
    queued_operation queued;
    queued.m_ctx = ctx;
    queued.m_op = op;
    return queued;
}

template<typename... Params>
queued_parameterized_operation<Params...>
queue_operation_with_target(const parameterized_operation_ptr<Params...>& op,
                            void* target)
{ /*
     queued_parameterized_operation<Params...> queued;
     queued.m_ctx.m_target = target;
     queued.m_op = op;
     return queued;
  */
    return op;
}

template queued_parameterized_operation<size_t>
queue_operation_with_target<size_t>(const iteration_operation_ptr&, void*);

template queued_parameterized_operation<socket_t, notifier_type>
queue_operation_with_target<socket_t, notifier_type>(
  const std::shared_ptr<socket_notifier_operation>&,
  void*);

void
execute_operation_on_this_thread(const queued_operation& op)
{
    void* previous = current_target;
    if (op.m_ctx.m_target) {
        current_target = op.m_ctx.m_target;
    }
    (*op.m_op)();
    current_target = previous;
}

void
execute_operation_on_this_thread(const operation_ptr& op)
{
    auto local = queue_operation_with_target(op, nullptr);
    execute_operation_on_this_thread(local);
}

template<typename... Params>
void
execute_operation_on_this_thread(
  const queued_parameterized_operation<Params...>& op,
  Params... params)
{
    /*void* previous = current_target;
    if (op.m_ctx.m_target) {
        current_target = op.m_ctx.m_target;
    }
    (*op.m_op)(params...);
    current_target = previous;*/
    (*op)(params...);
}
/*
template<typename... Params>
XDISPATCH_EXPORT void
execute_operation_on_this_thread(
  const parameterized_operation_ptr<Params...>& op,
  Params... params)
{
    auto local = queue_operation_with_target(op, nullptr);
    execute_operation_on_this_thread(local, params...);
} */

template void
execute_operation_on_this_thread<size_t>(
  const queued_parameterized_operation<size_t>&,
  size_t);
/*
template void
execute_operation_on_this_thread<size_t>(
  const parameterized_operation_ptr<size_t>&,
  size_t);*/

template void
execute_operation_on_this_thread<socket_t, notifier_type>(
  const queued_parameterized_operation<socket_t, notifier_type>&,
  socket_t,
  notifier_type);
/*
template void
execute_operation_on_this_thread<socket_t, notifier_type>(
  const parameterized_operation_ptr<socket_t, notifier_type>&,
  socket_t,
  notifier_type);*/

bool
operation_is_run_with_target(void const* const d)
{
    return (d == current_target);
}

__XDISPATCH_END_NAMESPACE
