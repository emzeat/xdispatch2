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
namespace naive
{

class apply_operation : public operation
{
public:
    apply_operation(
        size_t index,
        const iteration_operation_ptr& op,
        const consumable_ptr& consumable = consumable_ptr()
    );

    void operator()() final;

private:
    const size_t m_index;
    const iteration_operation_ptr m_op;
    const consumable_ptr m_consumable;
};

class delayed_operation : public operation
{
public:
    delayed_operation(
        std::chrono::milliseconds delay,
        const operation_ptr& op,
        const consumable_ptr& consumable = consumable_ptr()
    );

    void operator()() final;

private:
    const std::chrono::milliseconds m_delay;
    const operation_ptr m_op;
    const consumable_ptr m_consumable;
};

class consuming_operation : public operation
{
public:
    consuming_operation(
        const operation_ptr& op,
        const consumable_ptr& consumable
    );

    void operator()() final;

private:
    const operation_ptr m_op;
    const consumable_ptr m_consumable;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_OPERATIONS_H_ */