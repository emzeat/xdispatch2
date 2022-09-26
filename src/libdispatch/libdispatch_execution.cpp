/*
 * libdispatch_execution.cpp
 *
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

#include "libdispatch_execution.h"
#include "../xdispatch_internal.h"
#include "../trace_utils.h"
#include "../thread_utils.h"

#include "dispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

inline void
set_debugger_threadname_from_queue()
{
    if (trace_utils::is_debug_enabled()) {
        thread_utils::set_current_thread_name(
          dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL));
    }
}

void
run_wrapper(operation_wrap* wrapper)
{
    XDISPATCH_ASSERT(wrapper);

    const auto& wrappedOp = wrapper->type();

#if !(defined DEBUG)
    try
#endif
    {
        set_debugger_threadname_from_queue();
        execute_operation_on_this_thread(wrappedOp);
    }
#if !(defined DEBUG)
    catch (const std::exception& e) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before:\n"
                  << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;

        throw;
    } catch (...) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before!"
                  << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;

        std::terminate();
    }
#endif
}

extern "C" void
_xdispatch2_run_wrap_delete(void* dt)
{
    XDISPATCH_ASSERT(dt);

    std::unique_ptr<operation_wrap> wrapper(static_cast<operation_wrap*>(dt));
    run_wrapper(wrapper.get());
} // _xdispatch_run_operation

extern "C" void
_xdispatch2_run_wrap(void* dt)
{
    XDISPATCH_ASSERT(dt);

    auto* wrapper(static_cast<operation_wrap*>(dt));
    run_wrapper(wrapper);
} // _xdispatch_run_operation

extern "C" void
_xdispatch2_run_iter_wrap(void* dt, size_t index)
{
    XDISPATCH_ASSERT(dt);

    // note: dispatch_apply_f is a blocking call and as such the wrap will be
    // held on the stack,
    //       no ownership transfer into here
    auto* wrapper = static_cast<iteration_operation_wrap*>(dt);
    XDISPATCH_ASSERT(wrapper);

    const auto& wrapped_op = wrapper->type();

#if !(defined DEBUG)
    try
#endif
    {
        set_debugger_threadname_from_queue();
        execute_operation_on_this_thread(wrapped_op, index);
    }
#if !(defined DEBUG)
    catch (const std::exception& e) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before:\n"
                  << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        throw;
    } catch (...) {
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::cerr << "xdispatch: Throwing exceptions within an "
                     "xdispatch::operation is"
                  << std::endl;
        std::cerr << "           not recommended, please make sure to catch "
                     "them before!"
                  << std::endl;
        std::cerr << "#########################################################"
                     "#########"
                  << std::endl;
        std::terminate();
    }
#endif
} // _xdispatch_run_iter_wrap

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE
