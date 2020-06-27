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

#include "xdispatch_internal.h"
#include "xdispatch/itimer_impl.h"

#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    // #include "libdispatch/backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    // #include "libdispatch/backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    #include "libdispatch/backend_internal.h"
#endif

__XDISPATCH_BEGIN_NAMESPACE

static ibackend& platform_backend()
{
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    static libdispatch::backend s_backend;
    return s_backend;
#else
# error "No backend on this platform"
#endif
}

constexpr const char k_label_main[] = "de.mlba-team.xdispatch2.main";
constexpr const char k_label_global_low[] = "de.mlba-team.xdispatch2.low";
constexpr const char k_label_global_default[] = "de.mlba-team.xdispatch2.default";
constexpr const char k_label_global_high[] = "de.mlba-team.xdispatch2.high";

queue main_queue()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_main_queue( k_label_main );
    return queue( k_label_main, s_instance );
}

static queue global_queue_low()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_low, queue_priority::LOW );
    return queue( k_label_global_low, s_instance );
}

static queue global_queue_default()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_default, queue_priority::DEFAULT );
    return queue( k_label_global_default, s_instance );
}

static queue global_queue_high()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_high, queue_priority::HIGH );
    return queue( k_label_global_high, s_instance );
}

queue global_queue(
    queue_priority p
)
{
    switch( p )
    {
    case queue_priority::LOW:
        return global_queue_low();
    case queue_priority::DEFAULT:
        return global_queue_default();
    case queue_priority::HIGH:
        return global_queue_high();
    }
}

queue create_queue(
    const std::string& label
)
{
    return queue( label, platform_backend().create_serial_queue( label ) );
}

queue current_queue()
{
    return queue( "", iqueue_impl_ptr() );
}

timer create_timer(
    std::chrono::milliseconds interval,
    const queue& target
)
{
    auto impl = platform_backend().create_timer( target.implementation() );
    XDISPATCH_ASSERT( impl );
    impl->interval( interval );
    return timer( impl, target );
}

group create_group()
{
    return group( platform_backend().create_group() );
}

void exec()
{
    platform_backend().exec();
}

__XDISPATCH_END_NAMESPACE
