/*
 * naive_group.cpp
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
#include "xdispatch/impl/iqueue_impl.h"

#include "naive_backend_internal.h"
#include "naive_consumable.h"
#include "naive_operations.h"
#include "naive_threadpool.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class group_impl
  : public igroup_impl
  , public std::enable_shared_from_this<group_impl>
{
public:
    group_impl(backend_type backend)
      : igroup_impl()
      , m_backend(backend)
      , m_consumable(std::make_shared<consumable>())
    {}

    ~group_impl() override = default;

    void async(const queued_operation& op, const iqueue_impl_ptr& q) final
    {
        const auto c = std::atomic_load(&m_consumable);
        XDISPATCH_ASSERT(c);
        c->add_resource();

        operation_ptr consuming = std::make_shared<consuming_operation>(op, c);
        q->async(std::move(consuming));
    }

    bool wait(std::chrono::milliseconds timeout) final
    {
        // swap the previous consumable with a new one that all operations
        // submitted after this call will be added to and which waits on the
        // previous consumable in a chain. Use a compare/exchange and retry
        // whenever the consumable was already swapped by another thread
        consumable_ptr old_c;
        consumable_ptr new_c;
        do {
            old_c = std::atomic_load(&m_consumable);
            new_c = std::make_shared<consumable>(0, old_c);
        } while (
          !std::atomic_compare_exchange_weak(&m_consumable, &old_c, new_c));
        XDISPATCH_ASSERT(old_c);
        XDISPATCH_ASSERT(new_c);
        return old_c->wait_for_consumed(timeout);

        // FIXME(zwicker): This is blocking and will not work if invoked from
        // within
        //                 an operation active on the same queue as one of the
        //                 operations listed in the consumable
    }

    void notify(const queued_operation& op, const iqueue_impl_ptr& q) final
    {
        XDISPATCH_ASSERT(q);

        const auto this_ptr = shared_from_this();
        auto notify_op = make_operation([op, q, this_ptr] {
            // note: wait(..) will already notify the threadpool that
            //       we are blocking one of its threads through our internal
            //       use of a consumable
            this_ptr->wait(std::chrono::milliseconds::max());
            q->async(op);
        });

        // FIXME(zwicker): Add accessors to execute with the queue's priority
        threadpool::instance()->execute(notify_op, queue_priority::DEFAULT);
    }

    backend_type backend() final { return m_backend; }

private:
    const backend_type m_backend;
    consumable_ptr m_consumable;
};

igroup_impl_ptr
backend::create_group(backend_type backend)
{
    return std::make_shared<group_impl>(backend);
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
