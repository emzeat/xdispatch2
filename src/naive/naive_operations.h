/*
 * naive_operations.h
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

#include "../xdispatch_internal.h"
#include "naive_consumable.h"

#ifndef XDISPATCH_NAIVE_OPERATIONS_H_
    #define XDISPATCH_NAIVE_OPERATIONS_H_

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    @brief An operation grouping an iteration operation and
           invoking iterator together in a single call
 */
class apply_operation : public operation
{
public:
    /**
       @param index The iterator for which to call op
       @param op The iteration operation to be executed with index
       @param consumable The consumable to notify when done
     */
    apply_operation(size_t index,
                    const queued_parameterized_operation<size_t>& op,
                    const consumable_ptr& consumable = consumable_ptr());

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    const size_t m_index;
    const queued_parameterized_operation<size_t> m_op;
    const consumable_ptr m_consumable;
};

/**
    @brief An operation delaying the execution of
           another by using a provided timer

    The operation takes ownership of the timer and
    will cancel/release it when completed
 */
class delayed_operation : public operation
{
public:
    /**
       @brief Do not use, public to support std::make_shared
     */
    delayed_operation(itimer_impl_ptr&& timer,
                      const queued_operation& op,
                      const consumable_ptr& consumable = consumable_ptr());

    /**
       @param timer The timer used for delayed execution
       @param op The operation to be executed after delay
       @param consumable The consumable to notify when done
     */
    static void create_and_dispatch(itimer_impl_ptr&& timer,
                                    std::chrono::milliseconds delay,
                                    const queued_operation& op);

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    itimer_impl_ptr m_timer;
    const queued_operation m_op;
    const consumable_ptr m_consumable;
};

/**
    @brief An operation notifying a consumable when done
 */
class consuming_operation : public operation
{
public:
    /**
       @param op The operation to be executed
       @param consumable The consumable to notify when done
     */
    consuming_operation(const queued_operation& op,
                        const consumable_ptr& consumable);

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    const queued_operation m_op;
    const consumable_ptr m_consumable;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_OPERATIONS_H_ */
