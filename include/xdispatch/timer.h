/*
 * timer.h
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

#ifndef XDISPATCH_TIMER_H_
#define XDISPATCH_TIMER_H_

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

class itimer_impl;
using itimer_impl_ptr = std::shared_ptr<itimer_impl>;

/**
    @brief Configures the timer precision
 */
enum class timer_precision
{
    COARSE,
    DEFAULT,
    PRECISE
};

/**
  Provides a timer executing a lambda or an operation
  on a specific queue when a timeout occurs.
  */
class XDISPATCH_EXPORT timer
{
public:
    /**
        Constructs a new timer powered by the same backend as the target queue

        The timer will be stopped, call start() to execute it

        @param interval The interval at which the timer will fire after the
       timeout occured.
        @param target The queue to execute the timer on, defaults to the
       global_queue
    */
    explicit timer(std::chrono::milliseconds interval,
                   const queue& target = global_queue());

    /**
        Constructs a new periodic timer using the given implementation

        @param impl The implementation to be used
        @param target The queue on which a handler is executed after the timeout

        @throws std::logic_error if the impl backend is of a different type than
                the backend handling target
    */
    timer(const itimer_impl_ptr& impl, const queue& target);

    /**
        @brief Copy constructor
     */
    timer(const timer&) = default;

    /**
        @brief Move constructor
     */
    timer(timer&&) = default;

    /**
        @brief Destructor
     */
    ~timer() = default;

    /**
        @brief Use this to set the interval in nanoseconds.

        When called for the first time on a single-shot timer, the timer
        will be converted to a periodic timer with the given interval.
      */
    void interval(std::chrono::milliseconds interval);

    /**
        Use this to set the latency by which the timer
        might be early or late. When not set, a default latency will be used
      */
    void latency(timer_precision);

    /**
        Will start the timer.
        @remarks A new created timer will be stopped and needs to me started
       first. Once started, ensure balanced calls between start() and stop().

        Use the optional parameter to specify a time in nanoseconds after which
        the timer will fire for the first time. By default it will fire
       immediately and continue at the configured interval unless it was
       configured to be a singleshot timer.
    */
    void start(std::chrono::milliseconds delay = std::chrono::milliseconds(0));

    /**
      Will stop the timer.
      @remarks A new created timer will be stopped and needs to me started
      first. Once started, ensure balanced calls between start() and stop().
    */
    void stop();

    /**
        @brief assignment operator
     */
    timer& operator=(const timer&) = default;

    /**
        Sets the operation to dispatch onto the target queue whenever
        the timer becomes ready.
    */
    void handler(const operation_ptr& op);

    /**
        Sets the operation to dispatch onto the target queue whenever
        the timer becomes ready.
    */
    template<typename Func>
    inline void handler(const Func& f)
    {
        handler(make_operation(f));
    }

    /**
        @returns the queue the handler will be executed on
    */
    queue target_queue() const;

private:
    itimer_impl_ptr m_impl;
    queue m_target_queue;
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_TIMER_H_ */
