/*
 * cxx_dispatch_cascade_lambda.cpp
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

/*
Little tests mainly checking the correct mapping of the c++ api
to the underlying C Api
*/

void
cxx_dispatch_cascade_lambda(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_cascade_lambda);

    xdispatch::queue q = cxx_global_queue();

    int no = 0;

    q.async([=] {
        MU_ASSERT_EQUAL(no, 0);
        int no2 = no + 100;
        q.async([=] {
            MU_ASSERT_EQUAL(no2, 100);
            int no3 = no2 + 20;
            q.async([=] {
                MU_ASSERT_EQUAL(no3, 120);
                int no4 = no3 + 3;
                cxx_main_queue().async([=] {
                    MU_ASSERT_EQUAL(no4, 123);
                    MU_PASS("And Out");
                });
            });
        });
    });

    cxx_exec();
    MU_END_TEST;
}
