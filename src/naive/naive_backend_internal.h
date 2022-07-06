/*
 * naive_backend_internal.h
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

#ifndef XDISPATCH_NAIVE_INTERNAL_H_
#define XDISPATCH_NAIVE_INTERNAL_H_

#include "xdispatch/backend_naive.h"
#include "xdispatch/ibackend.h"

#include "../xdispatch_internal.h"

#include "naive_consumable.h"
#include "naive_operations.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class XDISPATCH_EXPORT backend : public ibackend
{
public:
    iqueue_impl_ptr create_main_queue(const std::string& label) override
    {
        return create_main_queue(label, backend_type::naive);
    }

    iqueue_impl_ptr create_serial_queue(const std::string& label,
                                        queue_priority priority) override
    {
        return create_serial_queue(label, priority, backend_type::naive);
    }

    iqueue_impl_ptr create_parallel_queue(const std::string& label,
                                          queue_priority priority) override
    {
        return create_parallel_queue(label, priority, backend_type::naive);
    }

    igroup_impl_ptr create_group() override
    {
        return create_group(backend_type::naive);
    }

    itimer_impl_ptr create_timer(const iqueue_impl_ptr& queue) override
    {
        return create_timer(queue, backend_type::naive);
    }

    isocket_notifier_impl_ptr create_socket_notifier(
      const iqueue_impl_ptr& queue,
      socket_t socket,
      notifier_type type) override
    {
        return create_socket_notifier(queue, socket, type, backend_type::naive);
    }

    backend_type type() const override { return backend_type::naive; }

    void exec() override;

protected:
    static iqueue_impl_ptr create_main_queue(const std::string& label,
                                             backend_type backend);

    static iqueue_impl_ptr create_serial_queue(const std::string& label,
                                               queue_priority priority,
                                               backend_type backend);

    static iqueue_impl_ptr create_parallel_queue(const std::string& label,
                                                 queue_priority priority,
                                                 backend_type backend);

    static igroup_impl_ptr create_group(backend_type backend);

    static itimer_impl_ptr create_timer(const iqueue_impl_ptr& queue,
                                        backend_type backend);

    static isocket_notifier_impl_ptr create_socket_notifier(
      const iqueue_impl_ptr& queue,
      socket_t socket,
      notifier_type type,
      backend_type backend);
};

queue
create_serial_queue(const std::string& label,
                    queue_priority priority,
                    backend_type backend = backend_type::naive);

queue
create_serial_queue(const std::string& label,
                    const ithreadpool_ptr& thread,
                    backend_type backend);

queue
create_serial_queue(const std::string& label,
                    const ithreadpool_ptr& threadpool,
                    queue_priority priority,
                    backend_type backend);

queue
create_parallel_queue(const std::string& label,
                      const ithreadpool_ptr& pool,
                      queue_priority priority,
                      backend_type backend);

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_INTERNAL_H_ */
