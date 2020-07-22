/*
* operation_manager.h
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

#ifndef XDISPATCH_NAIVE_OPERATION_MANAGER_H_
#define XDISPATCH_NAIVE_OPERATION_MANAGER_H_

#include <vector>

#include "naive_backend_internal.h"
#include "naive_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

using void_ptr = std::shared_ptr<void>;

class operation_queue_manager
{
public:
    ~operation_queue_manager();

    void attach(
        void_ptr q
    );

    void detach(
        void const* const q
    );

    static operation_queue_manager& instance();

private:
    operation_queue_manager();

    naive_thread m_thread;
    std::vector< void_ptr > m_queues;
};


}
__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_NAIVE_OPERATION_MANAGER_H_
