/*
 * platform_tests.cpp
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

#include <xdispatch/dispatch>

#include "platform_tests.h"

void
platform_test_main_queue(void*)
{
    MU_BEGIN_TEST(platform_test_main_queue);
    auto main = xdispatch::main_queue();

    main.async([] { MU_PASS("Platform main queue works"); });

    xdispatch::exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}

template<xdispatch::queue_priority p>
void
platform_test_global_queue(void*)
{
    MU_BEGIN_TEST(platform_test_global_queue);
    auto global = xdispatch::global_queue(p);

    global.async([] { MU_PASS("Platform global queue works"); });

    xdispatch::exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}

void
platform_test_custom_queue(void*)
{
    MU_BEGIN_TEST(platform_test_custom_queue);
    xdispatch::queue q("my queue");

    q.async([] { MU_PASS("Custom queue works"); });

    xdispatch::exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}

void
platform_test_timer(void*)
{
    MU_BEGIN_TEST(platform_test_timer);

    xdispatch::timer t(std::chrono::seconds(1));
    t.handler([] { MU_PASS("Platform timer works"); });
    t.start();

    xdispatch::exec();
    MU_END_TEST;
}

void
platform_test_group(void*)
{
    MU_BEGIN_TEST(platform_test_group);

    int passes = 0;

    xdispatch::group g;
    g.async([&] { ++passes; });
    MU_ASSERT_TRUE(g.wait());
    MU_ASSERT_EQUAL(passes, 1);
    MU_PASS("Platform group works");

    MU_END_TEST;
}

void
register_platform_tests()
{
    MU_REGISTER_TEST(platform_test_main_queue);
    MU_REGISTER_TEST(
      platform_test_global_queue<xdispatch::queue_priority::USER_INTERACTIVE>);
    MU_REGISTER_TEST(
      platform_test_global_queue<xdispatch::queue_priority::USER_INITIATED>);
    MU_REGISTER_TEST(
      platform_test_global_queue<xdispatch::queue_priority::UTILITY>);
    MU_REGISTER_TEST(
      platform_test_global_queue<xdispatch::queue_priority::DEFAULT>);
    MU_REGISTER_TEST(
      platform_test_global_queue<xdispatch::queue_priority::BACKGROUND>);
    MU_REGISTER_TEST(platform_test_custom_queue);
    MU_REGISTER_TEST(platform_test_timer);
    MU_REGISTER_TEST(platform_test_group);
}
