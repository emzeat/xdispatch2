/*
 * trace_utils.h
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

#ifndef XDISPATCH_TRACE_UTILS_H_
#define XDISPATCH_TRACE_UTILS_H_

#include <iostream>
#include <mutex>

#include "xdispatch_internal.h"
#include "xdispatch/impl/ibackend.h"

__XDISPATCH_BEGIN_NAMESPACE

class trace_utils
{
public:
    static bool is_trace_enabled();
    static bool is_debug_enabled();
    static void assert_same_backend(backend_type a, backend_type b);

private:
    trace_utils() = delete;
};

class trace_stream
{
public:
    inline trace_stream() { CS().lock(); }
    trace_stream(const trace_stream& other) = delete;

    ~trace_stream()
    {
        std::cerr << std::endl;
        CS().unlock();
    }

    template<typename T>
    inline trace_stream& operator<<(const T& type)
    {
        std::cerr << type;
        return *this;
    }

private:
    static std::mutex& CS();
};

#define XDISPATCH_TRACE_PREFIX "[xdispatch2] "

#define XDISPATCH_TRACE()                                                      \
    for (bool enabled = trace_utils::is_trace_enabled(); enabled;              \
         enabled = false)                                                      \
    trace_stream() << XDISPATCH_TRACE_PREFIX

#define XDISPATCH_WARNING() trace_stream() << XDISPATCH_TRACE_PREFIX

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_TRACE_UTILS_H_ */
