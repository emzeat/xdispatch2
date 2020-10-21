/*
* qt_tests.cpp
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

#include <xdispatch/backend_qt.h>
#include <xdispatch/signals.h>
#include <xdispatch/signals_barrier.h>

#include <QtCore/QObject>
#include <QtCore/QThread>

class TestObject : public QObject
{
public:
    TestObject()
        : QObject()
        , m_signal_value( 0 )
        , m_signal_calls( 0 )
    {
    }

    int m_signal_value;
    std::atomic<int> m_signal_calls;

public Q_SLOT:
    void myIntSignal( int value )
    {
        MU_ASSERT_EQUAL( value, m_signal_value );
        ++m_signal_calls;
    }
    void myIntSignal2( const int& value )
    {
        MU_ASSERT_EQUAL( value, m_signal_value );
        ++m_signal_calls;
    }
};

void qt_signal_connect( void* )
{
    MU_BEGIN_TEST( qt_signal_connect );

    xdispatch::signal<void( int )> int_signal;
    constexpr int k_signal_value = 11;

    auto obj = new TestObject;
    obj->m_signal_value = k_signal_value;
    xdispatch::qt5::connect( int_signal, obj, &TestObject::myIntSignal, xdispatch::global_queue() );
    xdispatch::qt5::connect( int_signal, obj, &TestObject::myIntSignal2, xdispatch::global_queue() );
    xdispatch::qt5::connect( int_signal, obj, [obj]( const int& value )
    {
        MU_ASSERT_EQUAL( value, obj->m_signal_value );
        ++obj->m_signal_calls;
    }, xdispatch::global_queue() );
    xdispatch::qt5::connect( int_signal, obj, [obj]( int value )
    {
        MU_ASSERT_EQUAL( value, obj->m_signal_value );
        ++obj->m_signal_calls;
    }, xdispatch::global_queue() );

    int_signal( k_signal_value );
    MU_SLEEP( 1 );
    MU_ASSERT_EQUAL( 4, obj->m_signal_calls );

    delete obj;
    obj = nullptr;
    int_signal( 0 );

    MU_PASS( "Disconnected" );
    MU_END_TEST;
}

void qt_custom_thread( void* )
{
    MU_BEGIN_TEST( qt_custom_thread );

    auto* thread = new QThread;
    thread->start();
    do
    {
        MU_SLEEP( 0 );
    }
    while( !thread->isRunning() );
    auto queue = xdispatch::qt5::create_serial_queue( "custom_thread", thread );

    queue.async( []
    {
        MU_PASS( "Executed" );
    } );

    xdispatch::exec();

    MU_FAIL( "Should not reach this" );
    MU_END_TEST;
}

void qt_custom_thread_not_started( void* )
{
    MU_BEGIN_TEST( qt_custom_thread_not_started );

    auto* thread = new QThread;
    auto queue = xdispatch::qt5::create_serial_queue( "custom_thread", thread );

    queue.async( []
    {
        MU_PASS( "Executed" );
    } );

    thread->start();
    xdispatch::exec();

    MU_FAIL( "Should not reach this" );
    MU_END_TEST;
}


void register_qt_tests()
{
    MU_REGISTER_TEST( qt_signal_connect );
    MU_REGISTER_TEST( qt_custom_thread );
    MU_REGISTER_TEST( qt_custom_thread_not_started );
}
