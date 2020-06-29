/*
* cxx_tests.cpp
*
* Copyright (c) 2008-2009 Apple Inc.
* Copyright (c) 2011-2013 MLBA-Team.
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

#include "cxx_tests.h"

void cxx_dispatch_group( void* );
void cxx_dispatch_mainqueue( void* );
void cxx_dispatch_timer( void* );
void cxx_dispatch_fibo( void* );
void cxx_dispatch_cascade_lambda( void* );
void cxx_dispatch_group_lambda( void* );
void cxx_dispatch_queue_lambda( void* );
void cxx_dispatch_serialqueue_lambda( void* );
void cxx_free_lambda( void* );

void register_cxx_tests(
    const char* name,
    xdispatch::ibackend* backend
)
{
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_group, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_mainqueue, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_timer, backend );
    //    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_fibo, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_cascade_lambda, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_group_lambda, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_queue_lambda, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_dispatch_serialqueue_lambda, backend );
    MU_REGISTER_TEST_INSTANCE( name, cxx_free_lambda, backend );
}

static xdispatch::ibackend* s_backend_tested = nullptr;

xdispatch::queue cxx_create_queue(
    const char* label
)
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_serial_queue( label );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::queue( label, impl );
}

xdispatch::queue cxx_global_queue(
    xdispatch::queue_priority priority
)
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_parallel_queue( "cxx_global_queue", priority );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::queue( "cxx_global_queue", impl );
}

xdispatch::queue cxx_main_queue()
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_main_queue( "cxx_main_queue" );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::queue( "cxx_main_queue", impl );
}

xdispatch::group cxx_create_group()
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_group();
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::group( impl );
}

void cxx_exec()
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    s_backend_tested->exec();
}

xdispatch::timer cxx_create_timer(
    const xdispatch::queue& queue
)
{
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_timer( queue.implementation() );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::timer( impl, queue );
}

void cxx_begin_test(
    void* data
)
{
    MU_ASSERT_NOT_NULL( data );
    s_backend_tested = static_cast< xdispatch::ibackend* >( data );
}
