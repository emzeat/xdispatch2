/*
 * backend_libdispatch.h
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

#ifndef XDISPATCH_BACKEND_LIBDISPATCH_H_
#define XDISPATCH_BACKEND_LIBDISPATCH_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"
#if (!BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    #error "The libdispatch backend is not available on this platform"
#endif

#include <dispatch/dispatch.h>

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

/**
    @return A new queue powered by the given dispatch_queue_t instance
    */
XDISPATCH_EXPORT queue
create_queue(dispatch_queue_t native);

/**
    @return A new group powered by the given dispatch_group_t instance
    */
XDISPATCH_EXPORT group
create_group(dispatch_group_t native);

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_BACKEND_LIBDISPATCH_H_ */
