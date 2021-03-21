/*
 * cxx_dispatch_mainqueue.cpp
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

#include <xdispatch/dispatch>
#include "cxx_tests.h"

#include <atomic>

#define RUN_TIMES 20

/*
 Little tests mainly checking the correct mapping of the c++ api
 to the underlying C Api
 */

class inc : public xdispatch::iteration_operation
{
public:
    explicit inc(std::atomic<int>* worker)
      : m_worker(worker)
    {}

    void operator()(size_t) final { (*m_worker)++; }

private:
    std::atomic<int>* m_worker;
};

class cleanup : public xdispatch::operation
{
public:
    explicit cleanup(std::atomic<int>* worker)
      : m_worker(worker)
    {}

    void operator()() final
    {
        MU_ASSERT_EQUAL(RUN_TIMES, m_worker->load());
        delete m_worker;
        MU_PASS("");
    }

private:
    std::atomic<int>* m_worker;
};

void
cxx_dispatch_mainqueue(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_mainqueue);

    auto* worker = new std::atomic<int>(0);

    xdispatch::queue q = cxx_main_queue();

    cxx_global_queue().apply(RUN_TIMES, std::make_shared<inc>(worker));
    q.async(std::make_shared<cleanup>(worker));

    cxx_exec();
    MU_END_TEST;
}
