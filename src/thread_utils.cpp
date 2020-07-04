/*
* thread_utils.cpp
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
#include "xdispatch/thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE

void thread_utils::set_current_thread_name(
    const std::string& name
)
{
#if (defined MZ_LINUX)
    prctl( PR_SET_NAME, ( unsigned long )( name.c_str() ), 0, 0, 0 ); // NOLINT(runtime/int)
#elif (defined MZ_MACOS || defined MZ_IOS)
    pthread_setname_np( name.c_str() );
#else
#   error "implement thread name";
#endif
}

__XDISPATCH_END_NAMESPACE
