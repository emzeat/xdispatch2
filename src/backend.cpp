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
#include "xdispatch/iqueue_impl.h"

#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    #include "naive/naive_backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    #include "qt5/qt5_backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    #include "libdispatch/libdispatch_backend_internal.h"
#endif

__XDISPATCH_BEGIN_NAMESPACE

static ibackend& backend_for_type(
    backend_type type
)
{
    switch( type )
    {
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    case backend_type::libdispatch:
        static libdispatch::backend s_backend_libdispatch;
        return s_backend_libdispatch;
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    case backend_type::qt5:
        static qt5::backend s_backend_qt5;
        return s_backend_qt5;
#endif
    default:
#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
        static naive::backend s_backend_naive;
        return s_backend_naive;
#endif
    }
}

static ibackend& platform_backend()
{
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    return backend_for_type( backend_type::libdispatch );
#elif (defined BUILD_XDISPATCH2_BACKEND_QT5)
    return backend_for_type( backend_type::qt5 );
#elif (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    return backend_for_type( backend_type::naive );
#else
# error "No backend on this platform"
#endif
}

queue main_queue()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_main_queue( k_label_main );
    return queue( k_label_main, s_instance );
}

static queue global_queue_USER_INTERACTIVE()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_low, queue_priority::USER_INTERACTIVE );
    return queue( k_label_global_low, s_instance );
}

static queue global_queue_USER_INITIATED()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_default, queue_priority::USER_INITIATED );
    return queue( k_label_global_default, s_instance );
}

static queue global_queue_UTILITY()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_default, queue_priority::UTILITY );
    return queue( k_label_global_default, s_instance );
}

static queue global_queue_BACKGROUND()
{
    static iqueue_impl_ptr s_instance = platform_backend().create_parallel_queue( k_label_global_high, queue_priority::BACKGROUND );
    return queue( k_label_global_high, s_instance );
}

queue global_queue(
    queue_priority p
)
{
    switch( p )
    {
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
}

queue::queue(
    const std::string& label,
    queue_priority priority
)
    : queue( label, platform_backend().create_serial_queue( label, priority ) )
{
}

timer::timer(
    std::chrono::milliseconds interval,
    const queue& target
)
    : timer( [interval, target]
{
    const auto q_type = target.implementation()->backend();
    auto impl = backend_for_type( q_type ).create_timer( target.implementation() );
    XDISPATCH_ASSERT( impl );
    impl->interval( interval );
    return timer( impl, target );
}
() )
{

}

group::group()
    : group( platform_backend().create_group() )
{
}

void exec()
{
    platform_backend().exec();
}

__XDISPATCH_END_NAMESPACE
