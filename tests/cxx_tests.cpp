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

#include <map>
#include <mutex>

void cxx_dispatch_group( void* );
void cxx_dispatch_mainqueue( void* );
void cxx_dispatch_timer( void* );
void cxx_dispatch_fibo( void* );
void cxx_dispatch_cascade_lambda( void* );
void cxx_dispatch_group_lambda( void* );
void cxx_dispatch_queue_lambda( void* );
void cxx_dispatch_serialqueue_lambda( void* );
void cxx_free_lambda( void* );
void cxx_is_current( void* );

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
    MU_REGISTER_TEST_INSTANCE( name, cxx_is_current, backend );
}

static std::mutex s_backend_CS;
static xdispatch::ibackend* s_backend_tested = nullptr;
static std::map< xdispatch::queue_priority, xdispatch::queue > s_backend_global_queues;
static std::unique_ptr< xdispatch::queue > s_backend_main_queue;

xdispatch::queue cxx_create_queue(
    const char* label
)
{
    std::lock_guard<std::mutex> lock( s_backend_CS );
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_serial_queue( label );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::queue( label, impl );
}

xdispatch::queue cxx_global_queue(
    xdispatch::queue_priority priority
)
{
    std::lock_guard<std::mutex> lock( s_backend_CS );
    MU_ASSERT_NOT_NULL( s_backend_tested );
    auto it = s_backend_global_queues.find( priority );
    if( it == s_backend_global_queues.end() )
    {
        const char* q_name = nullptr;
        switch( priority )
        {
        case xdispatch::queue_priority::LOW:
            q_name = "cxx_global_queue_LOW";
            break;
        case xdispatch::queue_priority::DEFAULT:
            q_name = "cxx_global_queue_DEFAULT";
            break;
        case xdispatch::queue_priority::HIGH:
            q_name = "cxx_global_queue_HIGH";
            break;
        }

        const auto impl = s_backend_tested->create_parallel_queue( q_name, priority );
        MU_ASSERT_NOT_NULL( impl.get() );
        it = s_backend_global_queues.emplace( std::make_pair( priority, xdispatch::queue( q_name, impl ) ) ).first;
    }
    return it->second;
}

xdispatch::queue cxx_main_queue()
{
    std::lock_guard<std::mutex> lock( s_backend_CS );
    MU_ASSERT_NOT_NULL( s_backend_tested );
    if( !s_backend_main_queue )
    {
        const auto impl = s_backend_tested->create_main_queue( "cxx_main_queue" );
        MU_ASSERT_NOT_NULL( impl.get() );
        s_backend_main_queue.reset( new xdispatch::queue( "cxx_main_queue", impl ) );
    }
    return *s_backend_main_queue;
}

xdispatch::group cxx_create_group()
{
    std::lock_guard<std::mutex> lock( s_backend_CS );
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
    std::lock_guard<std::mutex> lock( s_backend_CS );
    MU_ASSERT_NOT_NULL( s_backend_tested );
    const auto impl = s_backend_tested->create_timer( queue.implementation() );
    MU_ASSERT_NOT_NULL( impl.get() );
    return xdispatch::timer( impl, queue );
}

void cxx_begin_test(
    void* data
)
{
    std::lock_guard<std::mutex> lock( s_backend_CS );
    MU_ASSERT_NOT_NULL( data );
    s_backend_tested = static_cast< xdispatch::ibackend* >( data );
    s_backend_main_queue.reset();
    s_backend_global_queues.clear();
}
