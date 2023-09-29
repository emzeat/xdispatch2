/*
 * cxx_free_lambda.cpp
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

#include <xdispatch/dispatch>
#include "cxx_tests.h"

static std::atomic<int> s_loop_counter(0);
static std::atomic<int> s_ref_outer(0);
static std::atomic<int> s_ref_inner(0);

class ToBeFreed
{
public:
    explicit ToBeFreed(std::atomic<int>& ref_ct)
      : m_ref_ct(ref_ct)
    {
        ++m_ref_ct;
    }

    ToBeFreed(const ToBeFreed& other)
      : m_ref_ct(other.m_ref_ct)
    {
        ++m_ref_ct;
    }

    ToBeFreed(ToBeFreed&& other) noexcept
      : m_ref_ct(other.m_ref_ct)
    {
        ++m_ref_ct;
    }

    ~ToBeFreed() { --m_ref_ct; }

    void someFunction() const
    { /* Do nothing (tm) */
    }

private:
    std::atomic<int>& m_ref_ct;
};

static void
dispatch_outer()
{
    ToBeFreed outer(s_ref_outer);
    ToBeFreed inner(s_ref_inner);

    MU_ASSERT_EQUAL(s_ref_outer, 1);
    MU_ASSERT_EQUAL(s_ref_inner, 1);

    cxx_global_queue().apply(10, [inner](size_t) {
        inner.someFunction();
        ++s_loop_counter;
    });

    // apply returns as soon as the operation has been processed,
    // but it must not have been freed by then, this may still be
    // in progress. Hence give the system a moment to settle.
    MU_SLEEP(1);

    MU_ASSERT_EQUAL(s_ref_outer, 1);
    MU_ASSERT_EQUAL(s_ref_inner, 1);

    cxx_main_queue().async([outer] {
        MU_ASSERT_EQUAL(s_loop_counter, 10);
        MU_ASSERT_EQUAL(s_ref_inner, 0);
        outer.someFunction();
    });
}

void
cxx_free_lambda(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_free_lambda);

    dispatch_outer();
    cxx_main_queue().async([] {
        MU_ASSERT_EQUAL(s_loop_counter, 10);
        MU_ASSERT_EQUAL(s_ref_outer, 0);
        MU_ASSERT_EQUAL(s_ref_inner, 0);
        MU_PASS("Objects freed");
    });
    cxx_exec();

    MU_END_TEST;
}
