/*
 * ibackend.h
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

#ifndef XDISPATCH_IBACKEND_H_
#define XDISPATCH_IBACKEND_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
    The backends implemented on this platform
    */
enum class backend_type
{
#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    naive,
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    qt5,
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    libdispatch,
#endif
};

class XDISPATCH_EXPORT ibackend
{
public:
    virtual ~ibackend() = default;

    ibackend() = default;

    ibackend(const ibackend& other) = delete;

    virtual iqueue_impl_ptr create_main_queue(const std::string& label) = 0;

    virtual iqueue_impl_ptr create_serial_queue(const std::string& label,
                                                queue_priority priority) = 0;

    virtual iqueue_impl_ptr create_parallel_queue(const std::string& label,
                                                  queue_priority priority) = 0;

    virtual igroup_impl_ptr create_group() = 0;

    virtual itimer_impl_ptr create_timer(const iqueue_impl_ptr& queue) = 0;

    virtual isocket_notifier_impl_ptr create_socket_notifier(
      const iqueue_impl_ptr& target,
      socket_t socket,
      notifier_type type) = 0;

    virtual void exec() = 0;

    virtual backend_type type() const = 0;
};

__XDISPATCH_END_NAMESPACE

/** @} */

/** Helper macro for backend implementations to define their entry point */
#define XDISPATCH_DEFINE_BACKEND(type, class)                                  \
    extern "C" XDISPATCH_EXPORT xdispatch::ibackend*                           \
      type##_backend_get_static_instance_impl()                                \
    {                                                                          \
        static class _instance;                                                \
        return &_instance;                                                     \
    }

/** Helper macro to declare a backend entry point */
#define XDISPATCH_DECLARE_BACKEND(type)                                        \
    extern "C" xdispatch::ibackend* type##_backend_get_static_instance_impl(); \
    static inline xdispatch::ibackend* type##_backend_get_static_instance()    \
    {                                                                          \
        return type##_backend_get_static_instance_impl();                      \
    }

#endif /* XDISPATCH_IBACKEND_H_ */
