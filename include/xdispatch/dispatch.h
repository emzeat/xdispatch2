/*
 * dispatch.h
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

#ifndef XDISPATCH_H_
#define XDISPATCH_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "dispatch_decl.h"

#if defined(__cplusplus)

    #define __XDISPATCH_INDIRECT__
    #include "xdispatch/config.h"
    #include "xdispatch/operation.h"
    #include "xdispatch/queue.h"
    #include "xdispatch/backend.h"
    #include "xdispatch/group.h"
    #include "xdispatch/socket_notifier.h"
    #include "xdispatch/timer.h"
    #undef __XDISPATCH_INDIRECT__

#endif /* defined(__cplusplus) */

/** @} */

#endif /* XDISPATCH_H_ */
