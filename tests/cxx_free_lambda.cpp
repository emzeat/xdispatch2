/*
* cxx_free_lambda.cpp
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

static std::atomic<int> s_loop_counter( 0 );
static std::atomic<int> s_ref_outer( 0 );
static std::atomic<int> s_ref_inner( 0 );

class ToBeFreed
{
public:
    explicit ToBeFreed(
        std::atomic<int>& ref_ct
    )
        : m_ref_ct( ref_ct )
    {
        ++m_ref_ct;
    }

    ToBeFreed(
        const ToBeFreed& other
    ) : m_ref_ct( other.m_ref_ct )
    {
        ++m_ref_ct;
    }

    ~ToBeFreed()
    {
        --m_ref_ct;
    }

    void someFunction() const
    {
        /* Do nothing (tm) */
    }

private:
    std::atomic<int>& m_ref_ct;
};

static void dispatch_outer()
{
    ToBeFreed outer( s_ref_outer );
    ToBeFreed inner( s_ref_inner );

    cxx_global_queue().apply( 10, [ inner ]( size_t i )
    {
        inner.someFunction();
        ++s_loop_counter;
    } );

    cxx_main_queue().async( [ outer ]
    {
        MU_ASSERT_EQUAL( s_loop_counter, 10 );
        MU_ASSERT_EQUAL( s_ref_inner, 0 );
        outer.someFunction();
    } );
}

void cxx_free_lambda(
    void* data
)
{
    CXX_BEGIN_BACKEND_TEST( cxx_free_lambda );

    dispatch_outer();
    cxx_main_queue().async( []
    {
        MU_ASSERT_EQUAL( s_loop_counter, 10 );
        MU_ASSERT_EQUAL( s_ref_outer, 0 );
        MU_ASSERT_EQUAL( s_ref_inner, 0 );
        MU_PASS( "Objects freed" );
    });
    cxx_exec();

    MU_END_TEST;
}
