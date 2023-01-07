/*
 * backend.cpp
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

#include "xdispatch_internal.h"
#include "xdispatch/impl/itimer_impl.h"
#include "xdispatch/impl/isocket_notifier_impl.h"
#include "xdispatch/impl/iqueue_impl.h"

#include "symbol_utils.h"

/** Helper macro to import a backend entry point by resolving its symbol
 * globally, providing a fallback when unavailable */
#define XDISPATCH_IMPORT_SHARED_BACKEND(type, ...)                             \
    static inline xdispatch::ibackend* type##_backend_get_static_instance()    \
    {                                                                          \
        const char* backends[] = { #type, __VA_ARGS__ };                       \
        for (const char* backend : backends) {                                 \
            const auto symbol =                                                \
              std::string(backend) + "_backend_get_static_instance_impl";      \
            auto* instance =                                                   \
              xdispatch::symbol_utils::resolve<xdispatch::ibackend*(void)>(    \
                symbol.c_str());                                               \
            if (instance) {                                                    \
                return instance();                                             \
            }                                                                  \
        }                                                                      \
        XDISPATCH_WARNING() << "Failed to resolve backend '" << #type << "'";  \
        return nullptr;                                                        \
    }

#if (defined XDISPATCH2_BUILD_STATIC)
    #define XDISPATCH_IMPORT_BACKEND(type, ...) XDISPATCH_DECLARE_BACKEND(type);
#else
    #define XDISPATCH_IMPORT_BACKEND(type, ...)                                \
        XDISPATCH_IMPORT_SHARED_BACKEND(type, __VA_ARGS__);
#endif

#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
XDISPATCH_DECLARE_BACKEND(naive)
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
XDISPATCH_IMPORT_BACKEND(qt5, "libdispatch", "naive")
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
XDISPATCH_DECLARE_BACKEND(libdispatch)
#endif

__XDISPATCH_BEGIN_NAMESPACE

ibackend&
backend_for_type(backend_type type)
{
    switch (type) {
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
        case backend_type::libdispatch: {
            static ibackend& s_backend_libdispatch =
              *libdispatch_backend_get_static_instance();
            return s_backend_libdispatch;
        }
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
        case backend_type::qt5: {
            static ibackend& s_backend_qt5 = *qt5_backend_get_static_instance();
            return s_backend_qt5;
        }
#endif
        default:
#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
        {
            static ibackend& s_backend_naive =
              *naive_backend_get_static_instance();
            return s_backend_naive;
        }
#endif
    }
}

static ibackend&
platform_backend()
{
    // select backends based on compatibility and performance
    //
    // backends integrating with a UI framework always need to
    // go first as they might do magic to handle the main queue
    //
    // afterwards prefer the most efficient backend available
    // before falling back to the naive implementation
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    return backend_for_type(backend_type::qt5);
#elif (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    return backend_for_type(backend_type::libdispatch);
#elif (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    return backend_for_type(backend_type::naive);
#else
    #error "No backend on this platform"
#endif
}

queue
main_queue()
{
    static iqueue_impl_ptr s_instance =
      platform_backend().create_main_queue(k_label_main);
    return queue(k_label_main, s_instance);
}

static queue
global_queue_USER_INTERACTIVE()
{
    static iqueue_impl_ptr s_instance =
      platform_backend().create_parallel_queue(
        k_label_global_INTERACTIVE, queue_priority::USER_INTERACTIVE);
    return queue(k_label_global_INTERACTIVE, s_instance);
}

static queue
global_queue_USER_INITIATED()
{
    static iqueue_impl_ptr s_instance =
      platform_backend().create_parallel_queue(k_label_global_INITIATED,
                                               queue_priority::USER_INITIATED);
    return queue(k_label_global_INITIATED, s_instance);
}

static queue
global_queue_UTILITY()
{
    static iqueue_impl_ptr s_instance =
      platform_backend().create_parallel_queue(k_label_global_UTILITY,
                                               queue_priority::UTILITY);
    return queue(k_label_global_UTILITY, s_instance);
}

static queue
global_queue_BACKGROUND()
{
    static iqueue_impl_ptr s_instance =
      platform_backend().create_parallel_queue(k_label_global_BACKGROUND,
                                               queue_priority::BACKGROUND);
    return queue(k_label_global_BACKGROUND, s_instance);
}

queue
global_queue(queue_priority p)
{
    switch (p) {
        case queue_priority::USER_INTERACTIVE:
            return global_queue_USER_INTERACTIVE();
        case queue_priority::USER_INITIATED:
            return global_queue_USER_INITIATED();
        case queue_priority::DEFAULT:
        case queue_priority::UTILITY:
            return global_queue_UTILITY();
        case queue_priority::BACKGROUND:
            return global_queue_BACKGROUND();
    }
    assert(false && "Should never reach this");
    std::abort();
}

queue::queue(const std::string& label, queue_priority priority)
  : queue(label, platform_backend().create_serial_queue(label, priority))
{}

timer::timer(std::chrono::milliseconds interval, const queue& target)
  : timer([interval, &target] {
      const auto q_impl = target.implementation();
      const auto q_backend_type = q_impl->backend();
      const auto impl = backend_for_type(q_backend_type).create_timer(q_impl);
      XDISPATCH_ASSERT(impl);
      impl->interval(interval);
      return timer(impl, target);
  }())
{}

socket_notifier::socket_notifier(socket_t socket,
                                 notifier_type type,
                                 const queue& target)
  : socket_notifier([socket, type, &target] {
      const auto q_impl = target.implementation();
      const auto q_backend_type = q_impl->backend();
      const auto impl = backend_for_type(q_backend_type)
                          .create_socket_notifier(q_impl, socket, type);
      XDISPATCH_ASSERT(impl);
      return socket_notifier(impl, target);
  }())
{}

group::group()
  : group(platform_backend().create_group())
{}

void
exec()
{
    platform_backend().exec();
}

__XDISPATCH_END_NAMESPACE
