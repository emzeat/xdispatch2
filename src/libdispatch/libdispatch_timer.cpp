/*
 * libdispatch_timer.cpp
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

#include "xdispatch/itimer_impl.h"

#include "libdispatch_backend_internal.h"
#include "libdispatch_execution.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

class timer_impl : public itimer_impl
{
public:
    timer_impl(dispatch_source_t native)
      : itimer_impl()
      , m_native(native)
      , m_interval(0)
      , m_latency(0)
      , m_delay(DISPATCH_TIME_NOW)
    {
        XDISPATCH_ASSERT(m_native);
        dispatch_retain(m_native);
    }

    ~timer_impl() override
    {
        dispatch_source_cancel(m_native);
        dispatch_release(m_native);
        m_native = nullptr;
    }

    void interval(std::chrono::milliseconds interval) final
    {
        // changing the interval will also change the delay so that
        // the timer is not firing prematurely in case it is already
        // running and has its interval changed
        m_interval = interval.count() * NSEC_PER_MSEC;
        m_delay = dispatch_time(DISPATCH_TIME_NOW, std::int64_t(m_interval));
        dispatch_source_set_timer(m_native, m_delay, m_interval, m_latency);
    }

    void latency(timer_precision precision) final
    {
        switch (precision) {
            case timer_precision::COARSE:
                m_latency = NSEC_PER_SEC; // NOLINT(readability-magic-numbers)
                break;
            case timer_precision::DEFAULT:
                m_latency =
                  5 * NSEC_PER_MSEC; // NOLINT(readability-magic-numbers)
                break;
            case timer_precision::PRECISE:
                m_latency = 0; // NOLINT(readability-magic-numbers)
                break;
        }

        dispatch_source_set_timer(m_native, m_delay, m_interval, m_latency);
    }

    void handler(const operation_ptr& op) final
    {
        m_wrapper.reset(new operation_wrap(op));
        dispatch_set_context(m_native, m_wrapper.get());
        dispatch_source_set_event_handler_f(m_native, _xdispatch2_run_wrap);
    }

    void target_queue(const iqueue_impl_ptr& q) final
    {
        dispatch_set_target_queue(m_native, impl_2_native(q));
    }

    void start(std::chrono::milliseconds delay) final
    {
        if (0 == delay.count()) {
            m_delay = DISPATCH_TIME_NOW;
        } else {
            m_delay = dispatch_time(
              DISPATCH_TIME_NOW, std::int64_t(delay.count() * NSEC_PER_MSEC));
        }

        dispatch_source_set_timer(m_native, m_delay, m_interval, m_latency);
        dispatch_resume(m_native);
    }

    void stop() override { dispatch_suspend(m_native); }

    backend_type backend() final { return backend_type::libdispatch; }

private:
    dispatch_source_t m_native;
    uint64_t m_interval;
    uint64_t m_latency;
    uint64_t m_delay;
    std::unique_ptr<operation_wrap> m_wrapper;
};

itimer_impl_ptr
backend::create_timer(const iqueue_impl_ptr& queue)
{
    object_scope_T<dispatch_source_t> native(dispatch_source_create(
      DISPATCH_SOURCE_TYPE_TIMER, 0, 0, impl_2_native(queue)));
    return std::make_shared<timer_impl>(native.take());
}

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE
