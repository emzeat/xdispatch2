/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/


#include "execution.h"
#include "../xdispatch_internal.h"
#include "xdispatch/thread_utils.h"

#include "dispatch/dispatch.h"

#if (defined __linux__)
    #include <sys/prctl.h>
#endif

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch
{

#if (defined DEBUG)
inline void set_debugger_threadname_from_queue()
{
    thread_utils::set_current_thread_name( dispatch_queue_get_label( DISPATCH_CURRENT_QUEUE_LABEL ) );
}
#else
#  define set_debugger_threadname_from_queue()
#endif

void run_wrapper(
    operation_wrap* wrapper
)
{
    XDISPATCH_ASSERT( wrapper );

    const operation_ptr& wrappedOp = wrapper->type();
    XDISPATCH_ASSERT( wrappedOp );

#if !(defined DEBUG)
    try
#endif
    {
        set_debugger_threadname_from_queue();
        ( *wrappedOp )();
    }
#if !(defined DEBUG)
    catch( const std::exception& e )
    {
        std::cerr << "##################################################################" << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an xdispatch::operation is" << std::endl;
        std::cerr << "           not recommended, please make sure to catch them before:\n" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "##################################################################" << std::endl;

        throw;
    }
    catch( ... )
    {
        std::cerr << "##################################################################" << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an xdispatch::operation is" << std::endl;
        std::cerr << "           not recommended, please make sure to catch them before!" << std::endl;
        std::cerr << "##################################################################" << std::endl;

        std::terminate();
    }
#endif
}

extern "C"
void _xdispatch2_run_wrap_delete(
    void* dt
)
{
    XDISPATCH_ASSERT( dt );

    std::unique_ptr< operation_wrap > wrapper( static_cast< operation_wrap* >( dt ) );
    run_wrapper( wrapper.get() );
} // _xdispatch_run_operation

extern "C"
void _xdispatch2_run_wrap(
    void* dt
)
{
    XDISPATCH_ASSERT( dt );

    operation_wrap* wrapper( static_cast< operation_wrap* >( dt ) );
    run_wrapper( wrapper );
} // _xdispatch_run_operation

extern "C"
void _xdispatch2_run_iter_wrap(
    void* dt,
    size_t index
)
{
    XDISPATCH_ASSERT( dt );

    // note: dispatch_apply_f is a blocking call and as such the wrap will be held on the stack,
    //       no ownership transfer into here
    iteration_operation_wrap* wrapper = static_cast< iteration_operation_wrap* >( dt );
    XDISPATCH_ASSERT( wrapper );

    const iteration_operation_ptr& wrapped_op = wrapper->type();
    XDISPATCH_ASSERT( wrapped_op );

#if !(defined DEBUG)
    try
#endif
    {
        set_debugger_threadname_from_queue();
        ( *wrapped_op )( index );
    }
#if !(defined DEBUG)
    catch( const std::exception& e )
    {
        std::cerr << "##################################################################" << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an xdispatch::operation is" << std::endl;
        std::cerr << "           not recommended, please make sure to catch them before:\n" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "##################################################################" << std::endl;
        throw;
    }
    catch( ... )
    {
        std::cerr << "##################################################################" << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an xdispatch::operation is" << std::endl;
        std::cerr << "           not recommended, please make sure to catch them before!" << std::endl;
        std::cerr << "##################################################################" << std::endl;
        std::terminate();
    }
#endif
} // _xdispatch_run_iter_wrap

}
__XDISPATCH_END_NAMESPACE
