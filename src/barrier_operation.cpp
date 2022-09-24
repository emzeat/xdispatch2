/*
 * barrier_operation.cpp
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

#include "xdispatch/barrier_operation.h"

__XDISPATCH_BEGIN_NAMESPACE

barrier_operation::barrier_operation()
  : operation()
{}

bool
barrier_operation::wait(std::chrono::milliseconds timeout)
{
    return m_barrier.wait(timeout);
}

bool
barrier_operation::has_passed() const
{
    return m_barrier.was_completed();
}

void
barrier_operation::operator()()
{
    m_barrier.complete();
}

__XDISPATCH_END_NAMESPACE
