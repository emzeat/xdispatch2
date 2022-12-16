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
#include <xdispatch/barrier_operation.h>

#include "cxx_tests.h"
#include "stopwatch.h"

static Stopwatch s_checked_time;
static std::atomic_int s_scope_count{ 0 };
static constexpr std::chrono::milliseconds kPeriodicInterval(500);
template<typename Duration>
inline constexpr Duration
UpperBound(Duration target)
{
    return target * 7 / 6;
}
template<typename Duration>
inline constexpr Duration
LowerBound(Duration target)
{
    return target * 6 / 7;
}

struct ScopeCounter
{

    ScopeCounter() { ++s_scope_count; }
    ScopeCounter(const ScopeCounter&) { ++s_scope_count; }
    ScopeCounter(ScopeCounter&&) noexcept { ++s_scope_count; }
    ~ScopeCounter() { --s_scope_count; }
};

class test_periodic : public xdispatch::operation
{
public:
    test_periodic()
      : m_counter(0)
      , m_scope()
    {
        MU_ASSERT_EQUAL(s_scope_count, 1);
    }

    void operator()() final
    {
        // test the timer interval (but only after the second run
        // as the first one will be started immediately
        if (m_counter > 0) {
            const auto diff =
              std::chrono::duration_cast<std::chrono::milliseconds>(
                s_checked_time.elapsed());
            constexpr auto kTooEarly = LowerBound(kPeriodicInterval);
            constexpr auto kTooLate = UpperBound(kPeriodicInterval);
            MU_DESC_ASSERT_LESS_THAN(
              "timer not too late", diff.count(), kTooLate.count());
            MU_DESC_ASSERT_LESS_THAN(
              "timer not too early", kTooEarly.count(), diff.count());
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
    ScopeCounter m_scope;
};

void
cxx_dispatch_timer(xdispatch::timer& tested_timer)
{
    MU_MESSAGE("Testing periodic timer");
    tested_timer.interval(kPeriodicInterval);
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

void
cxx_dispatch_after(const xdispatch::queue& queue)
{
    bool async_complete = false;
    bool after_complete = false;

    Stopwatch elapsed;
    ScopeCounter counter;

    MU_ASSERT_EQUAL(s_scope_count.load(), 1);

    // dispatch two operations, one with delay, one without
    // with the one without delay expected to complete before
    // the other
    static constexpr auto kAfterInterval = std::chrono::milliseconds(500);

    queue.after(
      kAfterInterval,
      [&elapsed, &async_complete, &after_complete, counter, queue] {
          MU_ASSERT_TRUE(async_complete);
          MU_ASSERT_TRUE(!after_complete);
          after_complete = true;

          auto usec = std::chrono::duration_cast<std::chrono::milliseconds>(
            elapsed.elapsed());
          MU_ASSERT_LESS_THAN(usec.count(), UpperBound(kAfterInterval).count());
          MU_ASSERT_GREATER_THAN(usec.count(),
                                 LowerBound(kAfterInterval).count());

          MU_ASSERT_EQUAL(s_scope_count, 2);

          queue.async([] {
              MU_SLEEP(1);
              MU_ASSERT_EQUAL(s_scope_count, 1);
              MU_PASS("Done");
          });
      });
    queue.async([&async_complete, &after_complete, counter] {
        MU_ASSERT_TRUE(!async_complete);
        MU_ASSERT_TRUE(!after_complete);
        MU_ASSERT_EQUAL(s_scope_count, 3);
        async_complete = true;
    });
    elapsed.start();

    cxx_exec();
}

void
cxx_dispatch_after_serial(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_after_serial);

    auto tested_queue = cxx_create_queue("cxx_dispatch_after_serial");
    cxx_dispatch_after(tested_queue);

    MU_END_TEST;
}

void
cxx_dispatch_after_global(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_after_global);

    auto tested_queue = cxx_global_queue();
    cxx_dispatch_after(tested_queue);

    MU_END_TEST;
}

void
cxx_dispatch_after_main(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_after_main);

    auto tested_queue = cxx_main_queue();
    cxx_dispatch_after(tested_queue);

    MU_END_TEST;
}

void
cxx_dispatch_timer_suspend(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_timer_suspend);

    const std::chrono::milliseconds kInterval(500);
    auto timer = cxx_create_timer();
    timer.interval(kInterval);

    auto barrier = std::make_shared<xdispatch::barrier_operation>();
    timer.handler(barrier);
    timer.resume();
    MU_ASSERT_TRUE(barrier->wait(kInterval * 3 / 2));

    barrier = std::make_shared<xdispatch::barrier_operation>();
    timer.handler(barrier);
    timer.suspend();
    MU_ASSERT_TRUE(!barrier->wait(kInterval * 3 / 2));

    barrier = std::make_shared<xdispatch::barrier_operation>();
    timer.handler([&timer, barrier] {
        timer.suspend();
        if (barrier->has_passed()) {
            MU_FAIL("Should not hit this a second time");
        }
        (*barrier)();
    });
    timer.resume();
    MU_ASSERT_TRUE(barrier->wait(kInterval * 3));

    MU_PASS("");
}

void
cxx_dispatch_timer_cancel(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_timer_cancel);

    const std::chrono::milliseconds kInterval(500);
    auto timer = cxx_create_timer();
    timer.interval(kInterval);
    auto barrier = std::make_shared<xdispatch::barrier_operation>();
    timer.handler(barrier);
    timer.resume();
    timer.cancel();
    MU_ASSERT_TRUE(!barrier->wait(kInterval * 3 / 2));

    timer = cxx_create_timer();
    timer.interval(kInterval);
    barrier = std::make_shared<xdispatch::barrier_operation>();
    timer.handler([&timer, barrier] {
        timer.cancel();
        if (barrier->has_passed()) {
            MU_FAIL("Should not hit this a second time");
        }
        (*barrier)();
    });
    timer.resume();
    MU_ASSERT_TRUE(barrier->wait(kInterval * 3));

    MU_PASS("");
}
