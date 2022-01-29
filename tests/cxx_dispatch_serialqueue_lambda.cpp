/*
 * cxx_dispatch_serialqueue_lambda.cpp
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

#define JOBS_NO 20

#ifdef TARGET_OS_EMBEDDED
    #define LOOP_COUNT 2000000
#else
    #define LOOP_COUNT 100000
#endif

/*
 Little tests mainly checking the correct mapping of the c++ api
 to the underlying C Api
 */

void
cxx_dispatch_serialqueue_lambda(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_serialqueue_lambda);

    auto* worker = new std::atomic<unsigned int>(0);
    xdispatch::queue q = cxx_create_queue("cxx_dispatch_serialqueue");

    // dispatch some jobs
    for (unsigned int x = 0; x < JOBS_NO; x++) {
        q.async([=] {
            MU_ASSERT_EQUAL(*worker, x);
            // keep cpu busy
            for (int i = 0; i < LOOP_COUNT; i++) {
            }
            worker->store(x + 1);
        });
    }

    q.async([=] {
        MU_ASSERT_EQUAL(*worker, JOBS_NO);
        delete worker;
        // Test passed
        MU_PASS("Blocks were executed in correct order");
    });

    cxx_exec();
    MU_END_TEST;
}
