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

void
queue_operation_with_d(operation&, void*)
{}

template<typename... Params>
void
queue_operation_with_d(parameterized_operation<Params...>&, void*)
{}

template void
queue_operation_with_d<size_t>(iteration_operation&, void*);

template void
queue_operation_with_d<socket_t, notifier_type>(socket_notifier_operation&,
                                                void*);

void
execute_operation_on_this_thread(operation& op)
{
    op();
}

template<typename... Params>
XDISPATCH_EXPORT void
execute_operation_on_this_thread(parameterized_operation<Params...>& op,
                                 Params... params)
{
    op(params...);
}

template XDISPATCH_EXPORT void
execute_operation_on_this_thread<size_t>(iteration_operation&, size_t);

template XDISPATCH_EXPORT void
execute_operation_on_this_thread<socket_t, notifier_type>(
  socket_notifier_operation&,
  socket_t,
  notifier_type);

__XDISPATCH_END_NAMESPACE
