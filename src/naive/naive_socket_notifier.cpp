/*
 * naive_socket_notifier.cpp
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

#include "xdispatch/isocket_notifier_impl.h"
#include "xdispatch/iqueue_impl.h"

#include "naive_threadpool.h"
#include "naive_inverse_lockguard.h"
#include "../trace_utils.h"

#if (defined XDISPATCH2_HAVE_WINSOCK2)
    #include <winsock2.h>
#elif (defined XDISPATCH2_HAVE_SOCKETPAIR)
    #include <sys/select.h>
#else
    #error "naive_socket_notifier is not supported on this platform"
#endif
#include <cerrno>

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class socket_notifier_impl
  : public isocket_notifier_impl
  , public std::enable_shared_from_this<socket_notifier_impl>
{
public:
    socket_notifier_impl(const iqueue_impl_ptr& queue,
                         socket_t socket,
                         notifier_type type,
                         backend_type backend)
      : isocket_notifier_impl()
      , m_backend(backend)
      , m_socket(socket)
      , m_type(type)
      , m_queue(queue)
      , m_handler()
      , m_running(false)
    {}

    ~socket_notifier_impl() override { socket_notifier_impl::stop(); }

    void handler(const socket_notifier_operation_ptr& op) final
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_handler = op;
    }

    void target_queue(const iqueue_impl_ptr& q) final
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_queue = q;
    }

    void start() final
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_running = true;

        const auto this_ptr = shared_from_this();

        // the notifier will execute via a helper borrowed from the
        // global default threadpool. It is ensured that enough
        // threads are available for the pool even though the
        // notifier is blocking while it is active
        auto socket_notifier_op = make_operation([this_ptr] {
            threadpool::instance()->notify_thread_blocked();

            std::unique_lock<std::mutex> lock(this_ptr->m_CS);
            while (this_ptr->m_running) {
                const auto socket = this_ptr->m_socket;
                const auto type = this_ptr->m_type;

                int res = -1;
                {
                    inverse_lock_guard<std::mutex> unlock(this_ptr->m_CS);

                    struct timeval timeout;
                    constexpr auto k5sec = 5;
                    timeout.tv_sec = k5sec;
                    timeout.tv_usec = 0;

                    int nfds = static_cast<int>(socket) + 1;
                    fd_set fds;
                    memset(&fds, 0, sizeof(fds));
                    FD_SET(static_cast<int>(socket), &fds);

                    if (notifier_type::READ == type) {
                        res = select(nfds, &fds, nullptr, nullptr, &timeout);
                    } else {
                        res = select(nfds, nullptr, &fds, nullptr, &timeout);
                    }
                }

                if (!this_ptr->m_running) {
                    break;
                }

#if (defined XDISPATCH2_HAVE_WINSOCK2)
                if (SOCKET_ERROR == res && WSAENOTSOCK == WSAGetLastError()) {
                    // not a socket anymore
                    XDISPATCH_WARNING() << "socket_notifier: Socket " << socket
                                        << " is not a socket";
                    break;
                }
#elif (defined XDISPATCH2_HAVE_SOCKETPAIR)
                if (EBADF == res) {
                    // socket was closed somewhere else
                    XDISPATCH_WARNING()
                      << "socket_notifier: Socket " << socket << " is invalid";
                    break;
                }
#endif
                if (res == 0) {
                    // timeout, try again
                } else if (res > 0) {
                    XDISPATCH_TRACE() << "socket_notifier: select(" << socket
                                      << ") returned " << res;

                    const auto handler = this_ptr->m_handler;
                    const auto queue = this_ptr->m_queue;

                    barrier_operation barrier;
                    queue->async(
                      make_operation([handler, socket, type, &barrier] {
                          execute_operation_on_this_thread(
                            *handler, socket, type);
                          barrier();
                      }));
                    barrier.wait();
                } else {
                    XDISPATCH_WARNING() << "socket_notifier: select(" << socket
                                        << ") failed: " << strerror(errno);
                }
            }
            threadpool::instance()->notify_thread_unblocked();
        });

        // FIXME(zwicker): Add accessors to execute with the queue's priority
        threadpool::instance()->execute(socket_notifier_op,
                                        queue_priority::DEFAULT);
    }

    void stop() override
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_running = false;
    }

    socket_t socket() const final { return m_socket; }

    notifier_type type() const final { return m_type; }

    backend_type backend() final { return m_backend; }

private:
    const backend_type m_backend;
    std::mutex m_CS;
    const socket_t m_socket;
    const notifier_type m_type;
    iqueue_impl_ptr m_queue;
    socket_notifier_operation_ptr m_handler;
    bool m_running;
};

isocket_notifier_impl_ptr
backend::create_socket_notifier(const iqueue_impl_ptr& queue,
                                socket_t socket,
                                notifier_type type,
                                backend_type backend)
{
    return std::make_shared<socket_notifier_impl>(queue, socket, type, backend);
}

} // namespace naive
__XDISPATCH_END_NAMESPACE