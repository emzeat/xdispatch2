/*
 * libdispatch_backend_internal.h
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

#ifndef XDISPATCH_LIBDISPATCH_INTERNAL_H_
#define XDISPATCH_LIBDISPATCH_INTERNAL_H_

#include "xdispatch/backend_libdispatch.h"
#include "xdispatch/impl/ibackend.h"

#include "../xdispatch_internal.h"
#include "../naive/naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

class XDISPATCH_EXPORT backend : public naive::backend
{
public:
    iqueue_impl_ptr create_main_queue(const std::string& label) override;

    iqueue_impl_ptr create_serial_queue(const std::string& label,
                                        queue_priority priority) override;

    iqueue_impl_ptr create_parallel_queue(const std::string& label,
                                          queue_priority priority) override;

    // FIXME(zwicker): Implement efficient mixing of backends for groups
    // igroup_impl_ptr create_group() final;

    itimer_impl_ptr create_timer(const iqueue_impl_ptr& queue) override;

    isocket_notifier_impl_ptr create_socket_notifier(
      const iqueue_impl_ptr& queue,
      socket_t socket,
      notifier_type type) override;

    backend_type type() const override { return backend_type::libdispatch; }

    void exec() override { dispatch_main(); }
};

template<typename T>
class object_scope_T
{
public:
    object_scope_T(T object)
      : m_object(object)
    {}
    object_scope_T(const object_scope_T& other) = delete;

    ~object_scope_T()
    {
        if (m_object) {
            dispatch_release(m_object);
            m_object = nullptr;
        }
    }

    T take()
    {
        T tmp = nullptr;
        std::swap(tmp, m_object);
        return tmp;
    }

private:
    T m_object;
};

dispatch_queue_t
impl_2_native(const iqueue_impl_ptr&);

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_LIBDISPATCH_INTERNAL_H_ */
