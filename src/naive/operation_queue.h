/*
* operation_queue.h
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

#ifndef XDISPATCH_NAIVE_CONTEXTQUEUE_H_
#define XDISPATCH_NAIVE_CONTEXTQUEUE_H_

#include <list>
#include <mutex>

#include "backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

/**
 * Manages the access to an opengl context.
 *
 * When needing a current context, the work
 * is wrapped as functor or lambda and dispatched
 * on this queue using sync() or async(). It is
 * garuanteed the managed context is current when
 * the dispatched work is finally executed. So
 * there is no need to call makeCurrent() or
 * doneCurrent() manually.
 *
 * A separate thread automatically processing dispatched
 * items has to be launched using start(). When not using
 * such a thread, drain() can be used to process all
 * pending items. Behaviour is undefined when using drain()
 * after calling start() or when calling start() while
 * a call to drain() is active.
 */
class operation_queue : public std::enable_shared_from_this< operation_queue >
{
public:
    /**
     * Creates a new queue managing the access to
     * the given opengl context
     */
    explicit operation_queue(
        const ithread_ptr&
    );
    ~operation_queue();

    void async(
        const operation_ptr& job
    );

    void attach();

    void detach();

private:
    std::list< operation_ptr > m_jobs;
    std::mutex m_CS;
    operation_ptr m_notify_operation;
    ithread_ptr m_thread;

    void run();

    void process_job(
        operation& job
    );
};


using operation_queue_ptr = std::shared_ptr< operation_queue >;

}
__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_NAIVE_CONTEXTQUEUE_H_
