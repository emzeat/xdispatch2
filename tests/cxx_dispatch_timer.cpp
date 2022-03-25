/*
 * cxx_dispatch_timer.cpp
 *
 * Copyright (c) 2008 - 2009 Apple Inc.
 * Copyright (c) 2011 - 2022 Marius Zwicker
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

#include <xdispatch/dispatch.h>

#include "cxx_tests.h"
#include "stopwatch.h"

static Stopwatch s_checked_time;

class test_periodic : public xdispatch::operation
{
public:
    test_periodic()
      : m_counter(0)
    {}

    void operator()() final
    {
        // test the timer interval (but only after the second run
        // as the first one will be started immediately
        if (m_counter > 0) {
            const auto diff = s_checked_time.elapsed();
            constexpr auto k1second = 1.0 * 1000 * 1000;
            constexpr auto k3seconds = 3.0 * 1000 * 1000;
            MU_DESC_ASSERT_LESS_THAN_DOUBLE(
              "timer not too late", diff, k3seconds);
            MU_DESC_ASSERT_LESS_THAN_DOUBLE(
              "timer not too early", k1second, diff);
        }

        s_checked_time.start();

        // only pass when the timer fired at least 5 times
        MU_MESSAGE("\t%i", m_counter);
        if (m_counter < 5) {
            m_counter++;
        } else {
            MU_PASS("");
        }
    }

private:
    int m_counter;
};

void
cxx_dispatch_timer(xdispatch::timer& tested_timer)
{
    MU_MESSAGE("Testing periodic timer");
    tested_timer.interval(std::chrono::seconds(2));
    tested_timer.handler(std::make_shared<test_periodic>());
    s_checked_time.start();
    tested_timer.resume();
    cxx_exec();
}

void
cxx_dispatch_timer_main(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_timer_main);

    auto tested_timer = cxx_create_timer(cxx_main_queue());
    cxx_dispatch_timer(tested_timer);

    MU_END_TEST
}

void
cxx_dispatch_timer_global(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_timer_global);

    auto tested_timer = cxx_create_timer(cxx_global_queue());
    cxx_dispatch_timer(tested_timer);

    MU_END_TEST
}

void
cxx_dispatch_timer_serial(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_timer_serial);

    auto tested_timer =
      cxx_create_timer(cxx_create_queue("cxx_dispatch_timer"));
    cxx_dispatch_timer(tested_timer);

    MU_END_TEST
}
