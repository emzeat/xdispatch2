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

#ifndef XDISPATCH_NAIVE_INTERNAL_H_
#define XDISPATCH_NAIVE_INTERNAL_H_

#include "xdispatch/backend_naive.h"
#include "xdispatch/ibackend.h"

#include "../xdispatch_internal.h"

#include "consumable.h"
#include "operations.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class XDISPATCH_EXPORT backend : public ibackend
{
public:
    iqueue_impl_ptr create_main_queue(
        const std::string& label
    ) final;

    iqueue_impl_ptr create_serial_queue(
        const std::string& label
    ) final;

    iqueue_impl_ptr create_parallel_queue(
        const std::string& label,
        const queue_priority& priority
    ) final;

    igroup_impl_ptr create_group() final;

    itimer_impl_ptr create_timer(
        const iqueue_impl_ptr& queue
    ) final;

    backend_type type() const final
    {
        return backend_type::naive;
    }

    void exec() final;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_INTERNAL_H_ */
