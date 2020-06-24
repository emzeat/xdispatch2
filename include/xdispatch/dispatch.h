/*
* dispatch.h
*
* Copyright (c) 2011-2018 MLBA-Team
* All rights reserved.
*
* @LICENSE_HEADER_START@
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
* @LICENSE_HEADER_END@
*/


#ifndef XDISPATCH_H_
#define XDISPATCH_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "dispatch_decl.h"

#if defined ( __cplusplus )

    #define __XDISPATCH_INDIRECT__
    #include "xdispatch/config.h"
    #include "operation.h"
    #include "queue.h"
    #include "backend.h"
    #include "group.h"
    #include "timer.h"
    #undef __XDISPATCH_INDIRECT__

    #undef XDISPATCH_EXPORT

#endif /* defined(__cplusplus) */


/** @} */

#endif /* XDISPATCH_H_ */
