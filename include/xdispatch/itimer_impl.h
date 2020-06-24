/*
* itimer_impl.h
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


#ifndef XDISPATCH_ITIMER_IMPL_H_
#define XDISPATCH_ITIMER_IMPL_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
  @brief interface to be implemented to support a timer
  */
class itimer_impl
{
public:
    /**
        @brief Destructor
    */
    virtual ~itimer_impl() = default;

    /**
        @brief Use this to set the interval in nanoseconds.
      */
    virtual void interval(
        std::chrono::milliseconds interval
    ) = 0;

    /**
        Use this to set the latency by which the timer
        might be early or late.
      */
    virtual void latency(
        timer_precision
    ) = 0;

    /**
        Sets the operation to dispatch onto the target queue whenever
        the timer becomes ready.
    */
    virtual void handler(
        const operation_ptr& op
    ) = 0;

    /**
        Sets the queue the handler will be executed on
    */
    virtual void target_queue(
        const queue&
    ) = 0;

    /**
        Start the timer

        @param delay The time after which the timer will fire for the first time
      */
    virtual void start(
        std::chrono::milliseconds delay
    ) = 0;

    /**
        Will stop the timer.
    */
    virtual void stop() = 0;

private:
    itimer_impl(
        const itimer_impl&
    ) = delete;
};


__XDISPATCH_END_NAMESPACE


/** @} */

#endif /* XDISPATCH_ITIMER_IMPL_H_ */
