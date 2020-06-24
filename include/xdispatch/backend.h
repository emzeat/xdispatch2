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
    # error "Please #include <xdispatch/dispatch.h> instead of this file directly."
    # include "dispatch.h"
#endif

__XDISPATCH_BEGIN_NAMESPACE

class queue;
class timer;
class group;

/**
  The number of nanoseconds per second
  */
static constexpr std::chrono::nanoseconds nsec_per_sec = std::chrono::seconds( 1 );
/**
  The number of nanoseconds per millisecond
  */
static constexpr std::chrono::nanoseconds nsec_per_msec = std::chrono::milliseconds( 1 );
/**
  The number of nanoseconds per microsecond
  */
static const std::chrono::nanoseconds nsec_per_usec = std::chrono::microseconds( 1 );
/**
  The number of microseconds per second
  */
static const std::chrono::microseconds usec_per_sec = std::chrono::seconds( 1 );
/**
    Three priority classes used for the three standard
    global queues
    */
enum class queue_priority
{
    HIGH = 2, DEFAULT = 1, LOW = 0
};
/**
    The backends implemented on this platform
    */
enum class backend_type
{
#if (defined BUILD_XDISPATCH_BACKEND_NAIVE)
    naive,
#endif
#if (defined BUILD_XDISPATCH_BACKEND_QT5)
    qt,
#endif
#if (defined BUILD_XDISPATCH_BACKEND_LIBDISPATCH)
    libdispatch,
#endif
};

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
global_queue(
    queue_priority p = queue_priority::DEFAULT
);

/**
    @return A new queue powered by the platform default backend

    @param label The label to assign to the new queue
    */
XDISPATCH_EXPORT queue
create_queue(
    const std::string& label
);

/**
    @return A new timer powered by the platform default backend

    The timer will be stopped, call start() to execute it

    @param interval The interval at which the timer will fire after the timeout occured.
    @param target The queue to execute the timer on, defaults to the global_queue
    @param starting The time after which the timer will fire for the first time
    */
XDISPATCH_EXPORT timer
create_timer(
    std::chrono::milliseconds interval,
    const queue& target = global_queue(),
    std::chrono::milliseconds delay = std::chrono::milliseconds( 0 )
);

/**
    @return A new group powered by the platform default backend
    */
XDISPATCH_EXPORT group
create_group();

#if (defined BUILD_XDISPATCH_BACKEND_NAIVE)
# include "backend_libdispatch.h"
#endif
#if (defined BUILD_XDISPATCH_BACKEND_QT5)

#endif
#if (defined BUILD_XDISPATCH_BACKEND_LIBDISPATCH)

#endif

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_BACKEND_H_ */
