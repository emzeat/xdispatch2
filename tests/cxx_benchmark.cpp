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
#include <xdispatch/barrier_operation.h>
#include <atomic>

#include "cxx_tests.h"
#include "stopwatch.h"

constexpr int kCOUNT = 10000;

template<class receiver>
void
do_benchmark(receiver& r)
{
    Stopwatch watch_execution;
    Stopwatch watch_dispatch;
    std::atomic<int> passes(0);

    // begin measurement
    watch_execution.start();
    watch_dispatch.start();

    // schedule kCOUNT empty lambda blocks to measure the overhead
    // spent on scheduling the given queue
    auto work = xdispatch::make_operation([&passes] { ++passes; });
    for (int i = 0; i < kCOUNT; ++i) {
        r.async(work);
    }
    watch_dispatch.stop();

    auto barrier = std::make_shared<xdispatch::barrier_operation>();
    r.async(barrier);
    MU_ASSERT_TRUE(barrier->wait());

    watch_execution.stop();
    const int actual = passes;
    MU_MESSAGE("Dispatched %i operations, %i nsec per operation",
               actual,
               watch_dispatch.elapsed() * 1000 / actual);
    MU_MESSAGE("Executed %i operations, %i nsec per operation",
               actual,
               watch_execution.elapsed() * 1000 / actual);
}

void
cxx_benchmark_serial_queue(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_benchmark_serial_queue);

    auto queue = cxx_create_queue("serial_benchmark");
    do_benchmark(queue);

    MU_PASS("Test completed");
    MU_END_TEST;
}

void
cxx_benchmark_global_queue(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_benchmark_global_queue);

    auto queue = cxx_global_queue();
    do_benchmark(queue);

    MU_PASS("Test completed");
    MU_END_TEST;
}

void
cxx_benchmark_group(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_benchmark_group);

    auto group = cxx_create_group();
    auto queue = cxx_global_queue();

    Stopwatch watch;
    std::atomic<int> passes(0);

    // begin measurement
    watch.start();

    // schedule kCOUNT empty lambda blocks to measure the overhead
    // spent on scheduling the given queue
    auto work = xdispatch::make_operation([&passes] { ++passes; });
    for (int i = 0; i < kCOUNT; ++i) {
        group.async(work, queue);
    }

    // notify on completion
    group.notify(
      [&watch, &passes] {
          watch.stop();
          const int actual = passes;
          MU_MESSAGE("Dispatch %i operations, %i nsec per operation",
                     actual,
                     watch.elapsed() * 1000 / actual);
          MU_PASS("Test completed");
      },
      queue);

    cxx_exec();

    MU_FAIL("Should never reach this");
    MU_END_TEST;
}
