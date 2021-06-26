/*
 * cxx_waitable_queue.cpp
 *
 * Copyright (c) 2012-2021 MLBA-Team.
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

#include <list>

#include <xdispatch/waitable_queue.h>
#include <xdispatch/iqueue_impl.h>
#include "cxx_tests.h"

class manual_queue_impl : public xdispatch::iqueue_impl
{
public:
    void async(const xdispatch::operation_ptr& op) override
    {
        m_ops.push_back(op);
    }
    void apply(size_t, const xdispatch::iteration_operation_ptr&) override
    {
        MU_FAIL("Not implemented for this test");
    }
    void after(std::chrono::milliseconds,
               const xdispatch::operation_ptr&) override
    {
        MU_FAIL("Not implemented for this test");
    }
    xdispatch::backend_type backend() override
    {
        return static_cast<xdispatch::backend_type>(
          static_cast<int>(xdispatch::backend_type::naive) + 10);
    }

    void drain_one()
    {
        if (!m_ops.empty()) {
            auto op = m_ops.front();
            m_ops.pop_front();
            xdispatch::execute_operation_on_this_thread(*op);
        }
    }

private:
    std::list<xdispatch::operation_ptr> m_ops;
};

class manual_queue : public xdispatch::queue
{
public:
    manual_queue(const std::string& label)
      : xdispatch::queue(label, std::make_shared<manual_queue_impl>())
    {}

    void drain_one()
    {
        const auto inner =
          std::static_pointer_cast<manual_queue_impl>(implementation());
        return inner->drain_one();
    }
};

void
cxx_waitable_queue(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_waitable_queue);

    manual_queue inner("cxx_waitable_queue.inner");
    xdispatch::waitable_queue waitable("cxx_waitable_queue.outer", inner);

    // no op was queued so this should not block
    waitable.wait_for_one();
    waitable.wait_for_all();

    // queue an operation but have the inner queue not drain it, this
    // should force an implicit synchronous execution and hence not block
    bool executed = false;
    waitable.async([&] { executed = true; });
    waitable.wait_for_one();
    MU_ASSERT_TRUE(executed);
    // even draining the inner queue should now be a no-op
    executed = false;
    inner.drain_one();
    MU_ASSERT_TRUE(!executed);

    // queue an operation but have the inner queue drain it, this
    // should make the wait complete immediately.
    executed = false;
    waitable.async([&] { executed = true; });
    inner.drain_one();
    waitable.wait_for_one();
    MU_ASSERT_TRUE(executed);

    // queue an operation but destroy the outer queue before draining it
    // this should still be well defined
    {
        xdispatch::waitable_queue waitable2("cxx_waitable_queue.outer2", inner);
        executed = false;
        waitable2.async([&] { executed = true; });
    }
    inner.drain_one();
    MU_ASSERT_TRUE(executed);

    MU_PASS("Completed");
    MU_END_TEST;
}
