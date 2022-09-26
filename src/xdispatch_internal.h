/*
 * xdispatch_internal.h
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

#if XDISPATCH_COMPILER_MSVC2010 || XDISPATCH_COMPILER_MSVC2008SP1
    #pragma warning(                                                           \
      disable : 4251) /* disable warning C4251 - * requires dll-interface */
    #define XDISPATCH_EXPORT __declspec(dllexport)
    #define XDISPATCH_DEPRECATED(F) __declspec(deprecated) F
#elif XDISPATCH_COMPILER_GCC || XDISPATCH_COMPILER_CLANG
    #define XDISPATCH_EXPORT __attribute__((__visibility__("default")))
    #define XDISPATCH_DEPRECATED(F) __attribute__((__deprecated__)) F
#endif // if XDISPATCH_COMPILER_MSVC2010 || XDISPATCH_COMPILER_MSVC2008SP1

#include <assert.h>
#include <stdexcept>
#define XDISPATCH_ASSERT(X)                                                    \
    {                                                                          \
        if (!(X)) /* NOLINT(readability/braces) */                             \
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

struct queued_ctx
{
    void* m_target = nullptr;
};

class queued_operation
{
public:
    queued_operation() = default;
    inline queued_operation(const operation_ptr& op)
      : m_op(op)
      , m_ctx()
    {}

    operator bool() const
    {
        if (m_op) {
            return true;
        }
        return false;
    }

    inline void reset() { m_op.reset(); }

    operation_ptr m_op;
    queued_ctx m_ctx;
};

/*
template<typename... Params>
class queued_parameterized_operation
{
public:
    parameterized_queued_operation<Params...> m_op;
    queued_ctx m_ctx;
}; */

queued_operation
queue_operation_with_target(const operation_ptr&, void*);
queued_operation
queue_operation_with_ctx(const operation_ptr&, const queued_ctx&);
XDISPATCH_EXPORT void
execute_operation_on_this_thread(const queued_operation&);
bool
operation_is_run_with_target(void const* const);

ibackend&
backend_for_type(backend_type type);

__XDISPATCH_END_NAMESPACE

#undef __XDISPATCH_INDIRECT__

#endif /* XDISPATCH_INTERNAL_H_ */
