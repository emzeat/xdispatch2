/*
 * Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
 *
 * @MLBA_OPEN_LICENSE_HEADER_START@
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
 *
 * @MLBA_OPEN_LICENSE_HEADER_END@
 */

#include "xdispatch/dispatch.h"
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
                    const iteration_operation_ptr& op,
                    const consumable_ptr& consumable = consumable_ptr());

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    const size_t m_index;
    const iteration_operation_ptr m_op;
    const consumable_ptr m_consumable;
};

/**
    @brief An operation delaying the execution of
           another through simple blocking
 */
class delayed_operation : public operation
{
public:
    /**
       @param delay The time to block and hence delay op
       @param op The operation to be executed after delay
       @param consumable The consumable to notify when done
     */
    delayed_operation(std::chrono::milliseconds delay,
                      const operation_ptr& op,
                      const consumable_ptr& consumable = consumable_ptr());

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    const std::chrono::milliseconds m_delay;
    const operation_ptr m_op;
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
    consuming_operation(const operation_ptr& op,
                        const consumable_ptr& consumable);

    /**
        @copydoc operation::operator()()
     */
    void operator()() final;

private:
    const operation_ptr m_op;
    const consumable_ptr m_consumable;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_OPERATIONS_H_ */
