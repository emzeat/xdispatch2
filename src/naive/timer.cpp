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

#include <thread>

#include "xdispatch/itimer_impl.h"

#include "backend_internal.h"
#include "operations.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class timer_impl : public itimer_impl, public std::enable_shared_from_this<timer_impl>
{
public:
    explicit timer_impl(
        const iqueue_impl_ptr& queue
    ) : itimer_impl()
        , m_interval( 0 )
        , m_queue( queue )
        , m_handler()
        , m_running( false )
    {
    }

    ~timer_impl() final
    {
        stop();

        std::lock_guard< std::mutex > lock( m_CS );
        m_thread.detach();
    }

    void interval(
        std::chrono::milliseconds interval
    ) final
    {
        std::lock_guard< std::mutex > lock( m_CS );
        m_interval = interval;
    }

    void latency(
        timer_precision /* precision */
    ) final
    {
    }

    void handler(
        const operation_ptr& op
    ) final
    {
        std::lock_guard< std::mutex > lock( m_CS );
        m_handler = op;
    }

    void target_queue(
        const iqueue_impl_ptr& q
    ) final
    {
        std::lock_guard< std::mutex > lock( m_CS );
        m_queue = q;
    }

    void start(
        std::chrono::milliseconds delay
    ) final
    {
        std::lock_guard< std::mutex > lock( m_CS );
        m_running = true;

        const auto this_ptr = shared_from_this();
        std::thread thread( [this_ptr, delay]
        {
            std::this_thread::sleep_for( delay );

            std::unique_lock< std::mutex > lock( this_ptr->m_CS );
            while( this_ptr->m_running )
            {
                const auto handler = this_ptr->m_handler;
                const auto interval = this_ptr->m_interval;
                lock.unlock();

                handler->operator()();
                std::this_thread::sleep_for( interval );

                lock.lock();
            }
        } );

        std::swap( m_thread, thread );
    }

    void stop()
    {
        std::lock_guard< std::mutex > lock( m_CS );
        m_running = false;
    }

    backend_type backend() final
    {
        return backend_type::naive;
    }

private:
    std::mutex m_CS;
    std::chrono::milliseconds m_interval;
    iqueue_impl_ptr m_queue;
    operation_ptr m_handler;
    bool m_running;
    std::thread m_thread;
};

itimer_impl_ptr backend::create_timer(
    const iqueue_impl_ptr& queue
)
{
    return std::make_shared< timer_impl >( queue );
}

}
__XDISPATCH_END_NAMESPACE
