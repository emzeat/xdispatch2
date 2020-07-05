/*
* operation_queue.cpp
*
* Copyright (c) 2012-2018 Marius Zwicker
* All rights reserved.
*
* @LICENSE_HEADER_START@
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
* @LICENSE_HEADER_END@
*/

#include "operation_queue.h"
#include "operation_manager.h"
#include "naive_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

operation_queue::operation_queue(
    const ithread_ptr& thread
)
    : m_jobs()
    , m_CS()
    , m_notify_operation()
    , m_thread( thread )
{
}

operation_queue::~operation_queue()
{
    m_thread.reset();
    std::lock_guard<std::mutex> lock( m_CS );
}

void operation_queue::run()
{
    std::list< operation_ptr > jobs;
    {
        std::lock_guard<std::mutex> lock( m_CS );
        std::swap( m_jobs, jobs );
    }

    for( const operation_ptr& job : jobs )
    {
        process_job( *job );
    }
}


void operation_queue::async(
    const operation_ptr& job
)
{
    std::lock_guard<std::mutex> lock( m_CS );
    const bool notify = m_jobs.empty();
    m_jobs.push_back( job );
    if( notify && m_notify_operation )
    {
        m_thread->execute( m_notify_operation );
    }
}

void operation_queue::attach()
{
    class run_operation : public operation
    {
    public:
        explicit run_operation(
            operation_queue& q
        )
            : operation()
            , m_q( q )
        {
        }

        void operator()() final
        {
            m_q.run();
        }

    private:
        operation_queue& m_q;
    };

    std::lock_guard<std::mutex> lock( m_CS );
    m_notify_operation = std::make_shared<run_operation>( *this );

    const auto this_ptr = shared_from_this();
    XDISPATCH_ASSERT( this_ptr );
    operation_queue_manager::instance().attach( this_ptr );
}

void operation_queue::detach()
{
    {
        std::lock_guard<std::mutex> lock( m_CS );
        m_notify_operation.reset();
    }

    class detach_operation : public operation
    {
    public:
        explicit detach_operation(
            operation_queue const* const q
        )
            : operation()
            , m_q( q )
        {
        }

        void operator()() final
        {
            operation_queue_manager::instance().detach( m_q );
        }

    private:
        operation_queue const* const m_q;
    };

    async( std::make_shared<detach_operation>( this ) );
}


void operation_queue::process_job(
    operation& job
)
{
    execute_operation_on_this_thread( job );
}

}
__XDISPATCH_END_NAMESPACE
