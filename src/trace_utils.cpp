/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/

#include "trace_utils.h"

#include <cstdlib>

__XDISPATCH_BEGIN_NAMESPACE

static bool is_env_enabled(
    const char* env_variable
)
{
    const char* value = std::getenv( env_variable );
    if( value && 1 == std::atoi( value ) )
    {
        return true;
    }
    return false;
}

bool trace_utils::is_trace_enabled()
{
    static bool s_trace_enabled = is_env_enabled( "XDISPATCH2_TRACE" );
    return s_trace_enabled;
}

bool trace_utils::is_debug_enabled()
{
#if (defined DEBUG)
    return true;
#else
    return is_trace_enabled();
#endif
}

__XDISPATCH_END_NAMESPACE
