/*
 * qt_tests.cpp
 *
 * Copyright (c) 2008 - 2009 Apple Inc.
 * Copyright (c) 2011 - 2023 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 */

#include "cxx_tests.h"

#include <xdispatch/backend_qt.h>
#include <xdispatch/signals.h>
#include <xdispatch/signals_barrier.h>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>

class TestObject : public QObject
{
public:
    TestObject()
      : QObject()
      , m_signal_value(0)
      , m_signal_calls(0)
    {}

    int m_signal_value;              // NOLINT
    std::atomic<int> m_signal_calls; // NOLINT

    public // NOLINT
    Q_SLOT : void myIntSignal(int value)
    {
        MU_ASSERT_EQUAL(value, m_signal_value);
        ++m_signal_calls;
    }
    void myIntSignal2(const int& value)
    {
        MU_ASSERT_EQUAL(value, m_signal_value);
        ++m_signal_calls;
    }
};

void
qt_signal_connect(void*)
{
    MU_BEGIN_TEST(qt_signal_connect);

    xdispatch::signal<void(int)> int_signal;
    constexpr int k_signal_value = 11;

    auto* obj = new TestObject;
    obj->m_signal_calls = 0;
    obj->m_signal_value = k_signal_value;
    xdispatch::qt5::connect(
      int_signal, obj, &TestObject::myIntSignal, xdispatch::global_queue());
    xdispatch::qt5::connect(
      int_signal, obj, &TestObject::myIntSignal2, xdispatch::global_queue());
    xdispatch::qt5::connect(
      int_signal,
      obj,
      [obj](const int& value) {
          MU_ASSERT_EQUAL(value, obj->m_signal_value);
          ++obj->m_signal_calls;
      },
      xdispatch::global_queue());
    xdispatch::qt5::connect(
      int_signal,
      obj,
      [obj](int value) {
          MU_ASSERT_EQUAL(value, obj->m_signal_value);
          ++obj->m_signal_calls;
      },
      xdispatch::global_queue());

    int_signal(k_signal_value);
    MU_SLEEP(1);
    MU_ASSERT_EQUAL(4, obj->m_signal_calls);

    delete obj;
    obj = nullptr;
    int_signal(0);

    MU_PASS("Disconnected");
    MU_END_TEST;
}

void
qt_signal_disconnect(void*)
{
    MU_BEGIN_TEST(qt_signal_disconnect);

    xdispatch::signal<void(int)> int_signal;
    xdispatch::signal<void(int)> int_signal2;
    constexpr int k_signal_value = 15;

    auto* obj = new TestObject;
    obj->m_signal_calls = 0;
    obj->m_signal_value = k_signal_value;
    xdispatch::qt5::connect(
      int_signal, obj, &TestObject::myIntSignal, xdispatch::global_queue());
    xdispatch::qt5::connect(
      int_signal2, obj, &TestObject::myIntSignal2, xdispatch::global_queue());

    xdispatch::qt5::disconnect(int_signal, obj);

    // ensure connections were actually removed
    int_signal(k_signal_value);
    MU_SLEEP(1);
    MU_ASSERT_EQUAL(0, obj->m_signal_calls);

    // but connections to other signals remained
    int_signal2(k_signal_value);
    MU_SLEEP(1);
    MU_ASSERT_EQUAL(1, obj->m_signal_calls);

    // deleting the object should still be fine
    delete obj;
    obj = nullptr;
    int_signal(0);

    MU_PASS("Explict can be grouped with implicit disconnect");
    MU_END_TEST;
}

void
qt_custom_thread(void*)
{
    MU_BEGIN_TEST(qt_custom_thread);

    auto* thread = new QThread;
    thread->start();
    do {
        MU_SLEEP(0);
    } while (!thread->isRunning());
    auto queue = xdispatch::qt5::create_serial_queue("custom_thread", thread);

    queue.async([] { MU_PASS("Executed"); });

    xdispatch::exec();

    MU_FAIL("Should not reach this");
    MU_END_TEST;
}

void
qt_custom_thread_not_started(void*)
{
    MU_BEGIN_TEST(qt_custom_thread_not_started);

    auto* thread = new QThread;
    auto queue = xdispatch::qt5::create_serial_queue("custom_thread", thread);

    queue.async([] { MU_PASS("Executed"); });

    thread->start();
    xdispatch::exec();

    MU_FAIL("Should not reach this");
    MU_END_TEST;
}

template<class Application>
void
qt_application_t()
{
    char argv0[] = { 't', 'e', 's', 't', '\0' };
    char* argv[] = { &argv0[0] };
    int argc = 1;
    Application app(argc, argv);

    xdispatch::main_queue().async([] { MU_PASS("Executed on main"); });

    app.exec();

    MU_FAIL("Should not reach this");
}

void
qt_core_application(void*)
{
    MU_BEGIN_TEST(qt_core_application);
    qt_application_t<QCoreApplication>();
    MU_END_TEST;
}

void
qt_gui_application(void*)
{
    MU_BEGIN_TEST(qt_gui_application);
    qt_application_t<QGuiApplication>();
    MU_END_TEST;
}

void
qt_widgets_application(void*)
{
    MU_BEGIN_TEST(qt_widgets_application);
    qt_application_t<QApplication>();
    MU_END_TEST;
}

void
register_qt_tests()
{
    MU_REGISTER_TEST(qt_signal_connect);
    MU_REGISTER_TEST(qt_signal_disconnect);
    MU_REGISTER_TEST(qt_custom_thread);
    MU_REGISTER_TEST(qt_custom_thread_not_started);
    MU_REGISTER_TEST(qt_core_application);

#if (defined __APPLE__)
    // skip these tests on Linux by default as they cannot run headless
    MU_REGISTER_TEST(qt_gui_application);
    MU_REGISTER_TEST(qt_widgets_application);
#endif
}
