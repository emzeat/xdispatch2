/*
* signal_tests.cpp
*
* Copyright (c) 2008-2009 Apple Inc.
* Copyright (c) 2011-2020 MLBA-Team.
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

#include <xdispatch/dispatch.h>
#include <xdispatch/signals.h>
#include <xdispatch/signals_barrier.h>

#include "signal_tests.h"

void signal_test_void_connection(
    void*
)
{
    MU_BEGIN_TEST( signal_test_void_connection );

    xdispatch::signal<void( void )> void_signal;
    auto c = void_signal.connect( []
    {
        MU_PASS( "Works" );
    } );

    class SomeClass
    {
    public:
        void someFunc()
        {

        }
    };
    SomeClass someClass;
    auto c2 = void_signal.connect( &someClass, &SomeClass::someFunc );

    void_signal();

    xdispatch::exec();

    MU_FAIL( "Should never reach this" );
    c.disconnect();
    c2.disconnect();
    MU_END_TEST;
}


void signal_test_int_connection(
    void*
)
{
    MU_BEGIN_TEST( signal_test_int_connection );

    xdispatch::signal<void( int )> int_signal;
    auto c = int_signal.connect( [](
                                     int arg
                                 )
    {
        MU_ASSERT_EQUAL( 4, arg );
        MU_PASS( "Works" );
    } );

    class SomeClass
    {
    public:
        void someFunc(
            int arg
        )
        {

        }
    };
    SomeClass someClass;
    auto c2 = int_signal.connect( &someClass, &SomeClass::someFunc );

    int_signal( 4 );
    xdispatch::exec();

    MU_FAIL( "Should never reach this" );
    c.disconnect();
    c2.disconnect();
    MU_END_TEST;
}


void signal_test_disconnect(
    void*
)
{
    MU_BEGIN_TEST( signal_test_disconnect );

    xdispatch::signal<void( void )> void_signal;
    auto c = void_signal.connect( []
    {
        MU_FAIL( "Should never reach this" );
    } );

    c.disconnect();
    void_signal();

    MU_SLEEP( 5 );
    MU_PASS( "Works" );
}

void signal_test_disconnect_dangling(
    void*
)
{
    MU_BEGIN_TEST( signal_test_disconnect_dangling );

    auto void_signal = new xdispatch::signal<void( void )>;
    auto c = void_signal->connect( []
    {
        MU_FAIL( "Should never reach this" );
    } );

    delete void_signal;
    c.disconnect();
    MU_PASS( "Works" );
}

void signal_test_recursive_disconnect(
    void*
)
{
    MU_BEGIN_TEST( signal_test_disconnect );

    xdispatch::signal<void( void )> void_signal;
    xdispatch::connection_manager manager;
    manager += void_signal.connect( [&manager]
    {
        manager.reset_connections();
        MU_PASS( "Works" );
    } );

    void_signal();

    xdispatch::exec();
    MU_FAIL( "Should never reach this" );
    MU_END_TEST;
}

void signal_test_barrier(
    void*
)
{
    MU_BEGIN_TEST( signal_test_barrier );

    xdispatch::signal<void( void )> void_signal;
    xdispatch::signal_barrier<void( void )> barrier( void_signal );

    MU_ASSERT_NOT_TRUE( barrier.wait( std::chrono::milliseconds( 0 ) ) );
    xdispatch::global_queue().async( [&] { void_signal(); } );
    MU_ASSERT_TRUE( barrier.wait() );
    MU_ASSERT_TRUE( barrier.wait( std::chrono::milliseconds( 0 ) ) );

    MU_PASS( "Signal raised" );
    MU_END_TEST;
}

void signal_test_barrier_value(
    void*
)
{
    MU_BEGIN_TEST( signal_test_barrier_value );

    xdispatch::signal<void( int )> int_signal;
    xdispatch::signal_barrier<void( int )> barrier( int_signal );

    MU_ASSERT_NOT_TRUE( barrier.wait( std::chrono::milliseconds( 0 ) ) );
    try
    {
        MU_ASSERT_EQUAL( 0, barrier.value() );
        MU_FAIL( "Should raise std::runtime_error" );
    }
    catch( std::runtime_error& e )
    {
    }
    try
    {
        MU_ASSERT_EQUAL( 0, barrier.value<0>() );
    }
    catch( std::runtime_error& e )
    {
    }
    try
    {
        MU_ASSERT_EQUAL( 0, std::get<0>( barrier.values() ) );
    }
    catch( std::runtime_error& e )
    {
    }

    xdispatch::global_queue().async( [&] { int_signal( 24 ); } );
    MU_ASSERT_TRUE( barrier.wait() );
    MU_ASSERT_TRUE( barrier.wait( std::chrono::milliseconds( 0 ) ) );
    MU_ASSERT_EQUAL( 24, barrier.value() );
    MU_ASSERT_EQUAL( 24, barrier.value<0>() );
    MU_ASSERT_EQUAL( 24, std::get<0>( barrier.values() ) );

    int_signal( 80 );
    MU_ASSERT_EQUAL( 24, barrier.value() );
    MU_ASSERT_EQUAL( 24, barrier.value<0>() );
    MU_ASSERT_EQUAL( 24, std::get<0>( barrier.values() ) );

    MU_PASS( "Signal raised" );
    MU_END_TEST;
}

void register_signal_tests()
{
    MU_REGISTER_TEST( signal_test_void_connection );
    MU_REGISTER_TEST( signal_test_int_connection );
    MU_REGISTER_TEST( signal_test_disconnect );
    MU_REGISTER_TEST( signal_test_disconnect_dangling );
    MU_REGISTER_TEST( signal_test_recursive_disconnect );
    MU_REGISTER_TEST( signal_test_barrier );
    MU_REGISTER_TEST( signal_test_barrier_value );
}
