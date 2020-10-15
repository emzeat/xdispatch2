/*
* operation_queue_manager.cpp
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

#include "naive_operation_queue_manager.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

operation_queue_manager::~operation_queue_manager()
{
    m_thread.cancel();
}

void operation_queue_manager::attach(
    void_ptr q
)
{
    m_thread.execute( make_operation( [q, this]
    {
        m_queues.push_back( std::move( q ) );
    } ) );
}

void operation_queue_manager::detach(
    const void* const q
)
{
    m_thread.execute( make_operation( [q, this]
    {
        for( auto it = m_queues.begin(); it != m_queues.end(); )
        {
            if( it->get() == q )
            {
                it = m_queues.erase( it );
            }
            else
            {
                ++it;
            }
        }
    } ) );
}

operation_queue_manager& operation_queue_manager::instance()
{
    // remark: intentionally leak this object to ensure it outlives any other statics
    static operation_queue_manager* s_instance = new operation_queue_manager;
    return *s_instance;
}

operation_queue_manager::operation_queue_manager()
    : m_thread( "de.mlba-team.xdispatch2.op_q_manager", queue_priority::BACKGROUND )
{
}


}
__XDISPATCH_END_NAMESPACE
