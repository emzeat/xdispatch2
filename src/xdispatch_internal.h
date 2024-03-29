/*
 * xdispatch_internal.h
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
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

#ifndef XDISPATCH_INTERNAL_H_
#define XDISPATCH_INTERNAL_H_

#include <assert.h>
#include <string>
#include <iostream>

#define __XDISPATCH_BEGIN_NAMESPACE namespace xdispatch {
#define __XDISPATCH_END_NAMESPACE }
#define __XDISPATCH_USE_NAMESPACE                                              \
    using namespace xdispatch; // NOLINT(build/namespaces)

#ifndef __XDISPATCH_INDIRECT__
    #define __XDISPATCH_INDIRECT__
#endif

#include "../include/xdispatch/platform.h"
#include "../include/xdispatch/dispatch_decl.h"

#include <assert.h>
#include <stdexcept>
#define XDISPATCH_ASSERT(X)                                                    \
    {                                                                          \
        if (!(X)) /* NOLINT(readability/braces,                                \
                     readability-simplify-boolean-expr) */                     \
        {                                                                      \
            std::cerr << "Assertion failed: " #X " (at " << __FILE__ ":"       \
                      << __LINE__ << ")" << std::endl;                         \
            std::terminate();                                                  \
        }                                                                      \
    }

constexpr const char k_label_main[] = "de.emzeat.xdispatch2.main";
constexpr const char k_label_global_INTERACTIVE[] =
  "de.emzeat.xdispatch2.interactive";
constexpr const char k_label_global_INITIATED[] =
  "de.emzeat.xdispatch2.initiated";
constexpr const char k_label_global_UTILITY[] = "de.emzeat.xdispatch2.utility";
constexpr const char k_label_global_BACKGROUND[] =
  "de.emzeat.xdispatch2.background";

#include "xdispatch/config.h"
#include "../include/xdispatch/operation.h"
#include "../include/xdispatch/queue.h"
#include "../include/xdispatch/backend.h"
#include "../include/xdispatch/group.h"
#include "../include/xdispatch/timer.h"
#include "../include/xdispatch/socket_notifier.h"
#include "../include/xdispatch/impl/ibackend.h"

__XDISPATCH_BEGIN_NAMESPACE

void
queue_operation_with_d(operation&, void*);
void
execute_operation_on_this_thread(operation&);

ibackend&
backend_for_type(backend_type type);

__XDISPATCH_END_NAMESPACE

#undef __XDISPATCH_INDIRECT__

#endif /* XDISPATCH_INTERNAL_H_ */
