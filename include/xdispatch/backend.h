/*
 * backend.h
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

#ifndef XDISPATCH_BACKEND_H_
#define XDISPATCH_BACKEND_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#ifndef __XDISPATCH_INDIRECT__
    #error                                                                     \
      "Please #include <xdispatch/dispatch.h> instead of this file directly."
    #include "dispatch.h"
#endif

__XDISPATCH_BEGIN_NAMESPACE

class queue;
class timer;
class group;

/**
    Returns the main queue. This is the queue running
    within the main thread. Thus normally only items put
    on this queue can change the GUI.

    There will only be one main queue spawned by the default backend.
    */
XDISPATCH_EXPORT queue
main_queue();

/**
    Returns the global queue associated to the given
    Priority p

    operations submitted to these global concurrent queues
    may be executed concurrently with respect to
    each other.

    The default backend will always be used to support these queues.
    */
XDISPATCH_EXPORT queue
global_queue(queue_priority p = queue_priority::DEFAULT);

/**
    @brief Executes operations submitted to the main queue

    This function will never return.

    Depending on the backend it may not be necessary to invoke this,
    e.g. Qt will automatically drain its main queue
*/
XDISPATCH_EXPORT void
exec();

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_BACKEND_H_ */
