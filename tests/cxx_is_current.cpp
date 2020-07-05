/*
* cxx_dispatch_fibo.cpp
*
* Copyright (c) 2012 Simon Langevin
* Copyright (c) 2012-2013 MLBA-Team.
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


#include <xdispatch/dispatch>
#include "cxx_tests.h"

void cxx_is_current(
    void* data
)
{
    CXX_BEGIN_BACKEND_TEST( platform_test_main_queue );
    auto group = cxx_create_group();

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_main_queue().is_current_queue() );
    }, cxx_main_queue() );

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_global_queue( xdispatch::queue_priority::BACKGROUND ).is_current_queue() );
    }, cxx_global_queue( xdispatch::queue_priority::BACKGROUND ) );

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_global_queue( xdispatch::queue_priority::UTILITY ).is_current_queue() );
    }, cxx_global_queue( xdispatch::queue_priority::UTILITY ) );

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_global_queue( xdispatch::queue_priority::DEFAULT ).is_current_queue() );
    }, cxx_global_queue( xdispatch::queue_priority::DEFAULT ) );

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_global_queue( xdispatch::queue_priority::USER_INITIATED ).is_current_queue() );
    }, cxx_global_queue( xdispatch::queue_priority::USER_INITIATED ) );

    group.async( []
    {
        MU_ASSERT_TRUE( cxx_global_queue( xdispatch::queue_priority::USER_INTERACTIVE ).is_current_queue() );
    }, cxx_global_queue( xdispatch::queue_priority::USER_INTERACTIVE ) );

    group.notify( []
    {
        MU_PASS( "Seems to work" );
    }, cxx_main_queue() );

    cxx_exec();

    MU_FAIL( "Should never reach this" );
    MU_END_TEST;
}
