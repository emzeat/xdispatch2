/*
 * libdispatch_socket_notifier.cpp
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

#include <thread>

#include "xdispatch/isocket_notifier_impl.h"
#include "../trace_utils.h"

#include "libdispatch_backend_internal.h"
#include "libdispatch_execution.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

class socket_notifier_impl : public isocket_notifier_impl
{
public:
    socket_notifier_impl(const iqueue_impl_ptr& queue,
                         socket_t socket,
                         notifier_type type)
      : isocket_notifier_impl()
      , m_queue(queue)
      , m_socket(socket)
      , m_type(type)
      , m_native(nullptr)
    {
        const auto* source = type == notifier_type::READ
                               ? DISPATCH_SOURCE_TYPE_READ
                               : DISPATCH_SOURCE_TYPE_WRITE;
        m_native =
          dispatch_source_create(source, m_socket, 0, impl_2_native(m_queue));
    }

    ~socket_notifier_impl() override
    {
        if (m_native) {
            dispatch_resume(m_native);
            dispatch_source_cancel(m_native);
            dispatch_release(m_native);
            m_native = nullptr;
        }
    }

    void handler(const socket_notifier_operation_ptr& op) final
    {
        const auto socket = m_socket;
        const auto type = m_type;
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        const socket_notifier_operation_ptr op_strong_ref = op;
        dispatch_source_set_event_handler(m_native, ^{
          if (notifier_type::WRITE == type) {
              auto available = dispatch_source_get_data(m_native);
              if (0 == available) {
                  // FIXME(zwicker): Why do we get called with full buffer?
                  XDISPATCH_TRACE() << "socket_notifier: Write handler without "
                                       "available buffer";
                  static constexpr auto k50 = 50;
                  std::this_thread::sleep_for(std::chrono::milliseconds(k50));
                  return;
              }
          }
          execute_operation_on_this_thread(*op_strong_ref, socket, type);
        });
    }

    void target_queue(const iqueue_impl_ptr& q) final
    {
        m_queue = q;
        dispatch_set_target_queue(m_native, impl_2_native(q));
    }

    void resume() final { dispatch_resume(m_native); }

    void suspend() override { dispatch_suspend(m_native); }

    socket_t socket() const final { return m_socket; }

    notifier_type type() const final { return m_type; }

    backend_type backend() final { return backend_type::libdispatch; }

private:
    iqueue_impl_ptr m_queue;
    const socket_t m_socket;
    const notifier_type m_type;
    dispatch_source_t m_native;
};

isocket_notifier_impl_ptr
backend::create_socket_notifier(const iqueue_impl_ptr& queue,
                                socket_t socket,
                                notifier_type type)
{
    return std::make_shared<socket_notifier_impl>(queue, socket, type);
}

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE
