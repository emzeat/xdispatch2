/*
* base.cpp
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


#ifdef _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS 1
    #pragma warning (disable : 4996)
#endif

#include "xdispatch_internal.h"

__XDISPATCH_BEGIN_NAMESPACE

queue main_queue()
{
    return queue( "", iqueue_impl_ptr() );
}


queue global_queue(
    queue_priority p
)
{
    return queue( "", iqueue_impl_ptr() );
}

queue create_queue(
    const std::string &label
)
{
    return queue( "", iqueue_impl_ptr() );
}

timer create_timer(
    std::chrono::milliseconds interval,
    const queue &target,
    std::chrono::milliseconds delay
)
{
    return timer( itimer_impl_ptr(), target );
}

group create_group()
{
    return group( igroup_impl_ptr() );
}

__XDISPATCH_END_NAMESPACE
