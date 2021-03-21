/*
 * operation.cpp
 *
 * Copyright (c) 2011-2018 MLBA-Team
 * All rights reserved.
 *
 * @LICENSE_HEADER_START@
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
 * @LICENSE_HEADER_END@
 */

#include "xdispatch_internal.h"

__XDISPATCH_BEGIN_NAMESPACE

static thread_local void* current_d = nullptr;

void
queue_operation_with_d(operation& op, void* d)
{
    op.m_d = d;
}

void
queue_operation_with_d(iteration_operation& op, void* d)
{
    op.m_d = d;
}

void
execute_operation_on_this_thread(operation& op)
{
    void* previous = current_d;
    if (op.m_d) {
        current_d = op.m_d;
    }
    op();
    current_d = previous;
}

void
execute_operation_on_this_thread(iteration_operation& op, size_t index)
{
    void* previous = current_d;
    if (op.m_d) {
        current_d = op.m_d;
    }
    op(index);
    current_d = previous;
}

bool
operation_is_run_with_d(void const* const d)
{
    return (d == current_d);
}

operation::operation()
  : m_d(nullptr)
{}

iteration_operation::iteration_operation()
  : m_d(nullptr)
{}

__XDISPATCH_END_NAMESPACE
