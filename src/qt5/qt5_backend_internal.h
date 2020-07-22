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

#ifndef XDISPATCH_QT5_INTERNAL_H_
#define XDISPATCH_QT5_INTERNAL_H_

#include "xdispatch/backend_qt.h"
#if !(defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    # error "Naive backend is required to build qt5 backend"
#endif
#include "../naive/naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

class XDISPATCH_EXPORT backend : public naive::backend
{
public:
    iqueue_impl_ptr create_main_queue(
        const std::string& label
    ) final;

    iqueue_impl_ptr create_serial_queue(
        const std::string& label,
        queue_priority priority
    ) final;

    iqueue_impl_ptr create_parallel_queue(
        const std::string& label,
        queue_priority priority
    ) final;

    igroup_impl_ptr create_group() final;

    itimer_impl_ptr create_timer(
        const iqueue_impl_ptr& queue
    ) final;

    backend_type type() const final
    {
        return backend_type::qt5;
    }

    void exec() final;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_QT5_INTERNAL_H_ */
