/*
 * libdispatch_socket_notifier.cpp
 *
 * Copyright (c) 2011 - 2024 Marius Zwicker
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
#include <atomic>

#include "xdispatch/impl/isocket_notifier_impl.h"
#include "xdispatch/impl/iqueue_impl.h"
#include "xdispatch/impl/cancelable.h"
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

    static void static_notifier_callback(void* context)
    {
        auto* this_ptr = static_cast<socket_notifier_impl*>(context);
        this_ptr->notifier_callback();
    }

    void notifier_callback()
    {
        if (notifier_type::WRITE == m_type) {
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
        cancelable_scope scope(m_active);
        // no ordering constraints on m_notifier_operation it just needs to be
        // atomic
        const auto notifier = std::atomic_load_explicit(
          &m_notifier_operation, std::memory_order_relaxed);
        if (scope && notifier) {
            execute_operation_on_this_thread(*notifier, m_socket, m_type);
        }
    }

    void handler(const socket_notifier_operation_ptr& op) final
    {
        // we won't read m_notifier_operation, just make sure it is stored
        // before we update the handler and set the context
        std::atomic_store_explicit(
          &m_notifier_operation, op, std::memory_order_release);
        dispatch_set_context(m_native, this);
        dispatch_source_set_event_handler_f(m_native,
                                            &static_notifier_callback);
    }

    void target_queue(const iqueue_impl_ptr& q) final
    {
        m_queue = q;
        dispatch_set_target_queue(m_native, impl_2_native(q));
    }

    void resume() final { dispatch_resume(m_native); }

    void suspend() final { dispatch_suspend(m_native); }

    void cancel() final
    {
        dispatch_source_cancel(m_native);
        m_active.disable();
    }

    socket_t socket() const final { return m_socket; }

    notifier_type type() const final { return m_type; }

    backend_type backend() final { return backend_type::libdispatch; }

private:
    iqueue_impl_ptr m_queue;
    const socket_t m_socket;
    const notifier_type m_type;
    dispatch_source_t m_native;
    cancelable m_active;
    socket_notifier_operation_ptr m_notifier_operation;
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
