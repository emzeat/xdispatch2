/*
 * dispatch_priority.c
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

#include <cstdio>
#include <xdispatch/dispatch.h>
#include <cstdlib>
#include <cassert>
#include <atomic>
#include <vector>

#include "cxx_tests.h"

/*
 A test controlling that the different priorities
 really get different treatment
 */

static std::atomic<bool> done(false);

#define BLOCKS 128
#define PRIORITIES 4

#ifdef TARGET_OS_EMBEDDED
    #define LOOP_COUNT 2000000
#else
    #define LOOP_COUNT 20000000
#endif

static const char* labels[PRIORITIES] = { "BACKGROUND",
                                          "UTILITY",
                                          "USER_INITIATED",
                                          "USER_INTERACTIVE" };

static xdispatch::queue_priority priorities[PRIORITIES] = {
    xdispatch::queue_priority::BACKGROUND,
    xdispatch::queue_priority::UTILITY,
    xdispatch::queue_priority::USER_INITIATED,
    xdispatch::queue_priority::USER_INTERACTIVE
};

struct
{
    std::atomic<int> count;
    char padding[64];
} counts[PRIORITIES];

#define ITERATIONS static_cast<int>(PRIORITIES * BLOCKS * 0.5)
static std::atomic<int> iterations(ITERATIONS);

static void
histogram()
{
    size_t maxcount = BLOCKS;
    size_t sc[PRIORITIES];
    size_t total = 0;

    size_t x;
    size_t y;
    for (y = 0; y < PRIORITIES; ++y) {
        sc[y] = counts[y].count;
    }

    for (y = 0; y < PRIORITIES; ++y) {
        double fraction = 0;
        double value = 0;
        MU_MESSAGE("%s: %ld", labels[y], sc[y]);
        total += sc[y];

        fraction = (double)sc[y] / (double)maxcount;
        value = fraction * (double)80;
        for (x = 0; x < 80; ++x) {
            printf("%s", (value > x) ? "*" : " ");
        }
        printf("\n");
    }

    MU_ASSERT_EQUAL(total, ITERATIONS);
    // Do not consider this as failure, in order to remain efficient
    // the workload is pretty quick so it might complete faster then
    // being submitted on a high enough number of cores.
    // Rather use visual inspection instead.
    // MU_ASSERT_TRUE( sc[0] <= sc[PRIORITIES - 1] );
    MU_PASS("Please check histogram to be really sure");
}

static void
submit_work(const xdispatch::queue& queue, std::atomic<int>& count)
{
    int i;

    for (i = 0; i < BLOCKS; ++i) {
        queue.async([&count] {
            for (size_t idx = 0; idx < LOOP_COUNT; ++idx) {
                if (done.load()) {
                    return;
                }
            }

            const auto iterdone = --iterations;
            if (iterdone == 0) {
                done.store(true);
                ++count;
                histogram();
            } else if (iterdone > 0) {
                ++count;
            }
        });
    }
}

void
cxx_dispatch_priority_global(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_priority_global);

    std::vector<xdispatch::queue> q;
    q.reserve(PRIORITIES);
    for (int i = 0; i < PRIORITIES; i++) { // NOLINT(modernize-loop-convert)
        q.push_back(cxx_global_queue(priorities[i]));
    }

    for (int i = 0; i < PRIORITIES; i++) {
        submit_work(q[i], counts[i].count);
    }

    cxx_exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}

void
cxx_dispatch_priority_custom(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_priority_custom);

    std::vector<xdispatch::queue> q;
    q.reserve(PRIORITIES);
    for (int i = 0; i < PRIORITIES; i++) {
        q.push_back(cxx_create_queue(labels[i], priorities[i]));
    }

    for (int i = 0; i < PRIORITIES; i++) {
        submit_work(q[i], counts[i].count);
    }

    cxx_exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}
