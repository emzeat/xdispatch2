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

static std::atomic<bool> was_freed( false );
static std::atomic<int> loop_counter( 0 );

struct BeFreed2
{
    BeFreed2()
        : m_ref_ct( new std::atomic<int>( 1 ) )
    {
    }

    BeFreed2(
        const BeFreed2& other
    ) : m_ref_ct( other.m_ref_ct )
    {
        ( *m_ref_ct )++;
    }

    ~BeFreed2()
    {
        if( --( *m_ref_ct ) == 0 )
        {
            delete m_ref_ct;
            was_freed = true;
        }
    }

    void someFunction() const
    {
        /* Do nothing (tm) */
    }

private:
    std::atomic<int>* m_ref_ct;
};


struct BeFreed
{
    BeFreed()
        : m_ref_ct( new std::atomic<int>( 1 ) )
    {
    }

    BeFreed(
        const BeFreed& other
    ) : m_ref_ct( other.m_ref_ct )
    {
        ( *m_ref_ct )++;
    }

    ~BeFreed()
    {
        if( --( *m_ref_ct ) == 0 )
        {
            delete m_ref_ct;
            MU_ASSERT_TRUE( was_freed );
            MU_PASS( "" );
        }
    }

    void someFunction() const
    {
        /* Do nothing (tm) */
    }

private:
    std::atomic<int>* m_ref_ct;
};

static void dispatch_outer()
{
    BeFreed outer;
    BeFreed2 inner;

    cxx_global_queue().apply( 10, [ = ]( size_t i )
    {
        inner.someFunction();
        ++loop_counter;
    } );

    cxx_main_queue().async( [ = ]
    {
        MU_ASSERT_EQUAL( loop_counter, 10 );
        outer.someFunction();
    } );
}

void cxx_free_lambda(
    void* data
)
{
    CXX_BEGIN_BACKEND_TEST( cxx_free_lambda );

    dispatch_outer();
    cxx_exec();

    MU_END_TEST;
}
