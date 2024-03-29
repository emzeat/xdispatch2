/*
 * trace_utils.cpp
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

#include "trace_utils.h"

#include <cstdlib>

__XDISPATCH_BEGIN_NAMESPACE

static bool
is_env_enabled(const char* env_variable)
{
    const char* value = std::getenv(env_variable);
    return (value && 1 == std::atoi(value));
}

bool
trace_utils::is_trace_enabled()
{
    static bool s_trace_enabled = is_env_enabled("XDISPATCH2_TRACE");
    return s_trace_enabled;
}

bool
trace_utils::is_debug_enabled()
{
#if (defined DEBUG)
    return true;
#else
    return is_trace_enabled();
#endif
}

void
trace_utils::assert_same_backend(backend_type a, backend_type b)
{
    if (a != b) {
        const auto error = std::string("Cannot mix backends ") +
                           std::to_string(static_cast<int>(a)) + " and " +
                           std::to_string(static_cast<int>(b));
        std::cerr << XDISPATCH_TRACE_PREFIX << error << std::endl;
        XDISPATCH_ASSERT(false && "Cannot mix two different backends");
        throw std::logic_error(error);
    }
}

std::mutex&
trace_stream::CS()
{
    static auto* s_CS = new std::mutex;
    XDISPATCH_ASSERT(s_CS);
    return *s_CS;
}

__XDISPATCH_END_NAMESPACE
