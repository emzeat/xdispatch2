/*
 * libdispatch_group.cpp
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

#include "xdispatch/impl/igroup_impl.h"

#include "libdispatch_backend_internal.h"
#include "libdispatch_execution.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

class group_impl : public igroup_impl
{
public:
    group_impl(dispatch_group_t native)
      : igroup_impl()
      , m_native(native)
    {
        XDISPATCH_ASSERT(m_native);
        dispatch_retain(m_native);
    }

    ~group_impl() override
    {
        dispatch_release(m_native);
        m_native = nullptr;
    }

    void async(const queued_operation& op, const iqueue_impl_ptr& q) final
    {
        std::unique_ptr<operation_wrap> wrapper(new operation_wrap(op));
        dispatch_group_async_f(m_native,
                               impl_2_native(q),
                               wrapper.release(),
                               _xdispatch2_run_wrap_delete);
    }

    bool wait(std::chrono::milliseconds timeout) final
    {
        if (std::chrono::milliseconds::max() == timeout) {
            return 0 == dispatch_group_wait(m_native, DISPATCH_TIME_FOREVER);
        }
        if (0 == timeout.count()) {
            return 0 == dispatch_group_wait(m_native, DISPATCH_TIME_NOW);
        }

        const auto time = dispatch_time(
          DISPATCH_TIME_NOW, std::int64_t(timeout.count() * NSEC_PER_MSEC));
        return 0 == dispatch_group_wait(m_native, time);
    }

    void notify(const queued_operation& op, const iqueue_impl_ptr& q) final
    {
        std::unique_ptr<operation_wrap> wrapper(new operation_wrap(op));
        dispatch_group_notify_f(m_native,
                                impl_2_native(q),
                                wrapper.release(),
                                _xdispatch2_run_wrap_delete);
    }

    backend_type backend() final { return backend_type::libdispatch; }

private:
    dispatch_group_t m_native;
};

group
create_group(dispatch_group_t native)
{
    return group(std::make_shared<group_impl>(native));
}

// FIXME(zwicker): Implement efficient mixing of backends for groups
// igroup_impl_ptr backend::create_group()
//{
//    object_scope_T< dispatch_group_t > native( dispatch_group_create() );
//    return std::make_shared< group_impl >( native.take() );
//}

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE
