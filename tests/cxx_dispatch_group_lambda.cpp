/*
 * cxx_dispatch_group_lambda.cpp
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

#include <xdispatch/dispatch>
#include "cxx_tests.h"

#include <iostream>

/*
 Little tests mainly checking the correct mapping of the c++ api
 to the underlying C Api
 */

static xdispatch::group
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
create_group(size_t count, int delay)
{
    size_t i;

    auto group = cxx_create_group();

    for (i = 0; i < count; ++i) {
        auto queue = cxx_create_queue("cxx_dispatch_group_lambda");

        group.async(
          [=] {
              if (delay) {
                  MU_MESSAGE("sleeping...");
                  MU_SLEEP(delay);
                  MU_MESSAGE("done.");
              }
          },
          queue);
    }
    return group;
}

void
cxx_dispatch_group_lambda(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_group_lambda);

    xdispatch::group group = create_group(100, 0);
    group.wait();

    // should be OK to re-use a group
    group.async([=] {}, cxx_global_queue());
    group.wait();

    group = create_group(3, 7);
    bool res = group.wait(std::chrono::seconds(5));
    MU_ASSERT_EQUAL(res, false);
    // retry after timeout (this time succeed)
    res = group.wait(std::chrono::seconds(5));
    MU_ASSERT_EQUAL(res, true);

    group = create_group(100, 0);

    group.notify([=] { MU_PASS("Great!"); }, cxx_main_queue());

    cxx_exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST
}
