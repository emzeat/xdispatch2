/*
 * libdispatch_queue.cpp
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

#include "xdispatch/impl/iqueue_impl.h"
#include "../thread_utils.h"

#include "libdispatch_backend_internal.h"
#include "libdispatch_execution.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

class queue_impl : public iqueue_impl
{
public:
    queue_impl(dispatch_queue_t native)
      : iqueue_impl()
      , m_native(native)
    {
        XDISPATCH_ASSERT(m_native);
        dispatch_retain(m_native);
    }

    ~queue_impl() override
    {
        dispatch_release(m_native);
        m_native = nullptr;
    }

    void async(const operation_ptr& op) final
    {
        std::unique_ptr<operation_wrap> wrapper(new operation_wrap(op));
        dispatch_async_f(
          m_native, wrapper.release(), _xdispatch2_run_wrap_delete);
    }

    void apply(size_t times, const iteration_operation_ptr& op) final
    {
        iteration_operation_wrap wrap(op);

        dispatch_apply_f(times, m_native, &wrap, _xdispatch2_run_iter_wrap);
    }

    void after(std::chrono::milliseconds delay, const operation_ptr& op) final
    {
        const auto time = dispatch_time(
          DISPATCH_TIME_NOW, std::int64_t(delay.count() * NSEC_PER_MSEC));
        std::unique_ptr<operation_wrap> wrapper(new operation_wrap(op));
        dispatch_after_f(
          time, m_native, wrapper.release(), _xdispatch2_run_wrap_delete);
    }

    backend_type backend() final { return backend_type::libdispatch; }

    friend dispatch_queue_t impl_2_native(const iqueue_impl_ptr& impl);

private:
    dispatch_queue_t m_native;
};

dispatch_queue_t
impl_2_native(const iqueue_impl_ptr& impl)
{
    XDISPATCH_ASSERT(backend_type::libdispatch == impl->backend());
    auto& dispatch = static_cast<queue_impl&>(*impl);
    return dispatch.m_native;
}

queue
create_queue(dispatch_queue_t native)
{
    return queue(dispatch_queue_get_label(native),
                 std::make_shared<queue_impl>(native));
}

iqueue_impl_ptr
backend::create_main_queue(const std::string& /* label */
)
{
    return std::make_shared<queue_impl>(dispatch_get_main_queue());
}

iqueue_impl_ptr
backend::create_serial_queue(const std::string& label, queue_priority priority)
{
    dispatch_queue_attr_t qos_attr = dispatch_queue_attr_make_with_qos_class(
      DISPATCH_QUEUE_SERIAL, thread_utils::map_priority_to_qos(priority), 0);
    object_scope_T<dispatch_queue_t> native(
      dispatch_queue_create(label.c_str(), qos_attr));
    return std::make_shared<queue_impl>(native.take());
}

iqueue_impl_ptr
backend::create_parallel_queue(const std::string& /* label */,
                               queue_priority priority)
{
    const auto qos = thread_utils::map_priority_to_qos(priority);
    return std::make_shared<queue_impl>(dispatch_get_global_queue(qos, 0));
}

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE
