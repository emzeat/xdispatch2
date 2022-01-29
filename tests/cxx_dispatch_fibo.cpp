/*
 * cxx_dispatch_fibo.cpp
 *
 * Copyright (c) 2012 Simon Langevin
 * Copyright (c) 2012 - 2022 Marius Zwicker
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

class operation_fibo : public xdispatch::operation
{
public:
    explicit operation_fibo(int fiboNumber)
      : m_myFibo(fiboNumber)
    {}

    void operator()() final
    {
        if (m_myFibo < 2) {
            // nothing
        } else {
            xdispatch::group leGroupe = cxx_create_group();
            auto fiboA = std::make_shared<operation_fibo>(m_myFibo - 1);
            auto fiboB = std::make_shared<operation_fibo>(m_myFibo - 2);
            leGroupe.async(fiboA);
            leGroupe.async(fiboB);

            leGroupe.wait();

            // Dumb calculation to take some CPU time.
            float j = 1.0F / 3.0F;
            for (int i = 1; i < 30000; i++) {
                j = j * static_cast<float>(i);
            }
        }
    }

private:
    int m_myFibo;
};

void
cxx_dispatch_fibo(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_fibo);

    bool ticTac = true;
    xdispatch::group theGroup = cxx_create_group();
    xdispatch::operation_ptr theOps[6];
    int cpt = 0;
    while (cpt < 3000) {
        for (int i = 0; i < 6; i++) { // NOLINT(modernize-loop-convert)
            theOps[i] = std::make_shared<operation_fibo>(8);
            theGroup.async(theOps[i]);
        }
        theGroup.wait();

#if 0
        if( ticTac )
        {
            MU_MESSAGE( "Tic" );
        }
        else
        {
            MU_MESSAGE( "Tac" );
        }
#endif

        ticTac = !ticTac;
        cpt++;
    }

    MU_PASS("");
    MU_END_TEST;
}
