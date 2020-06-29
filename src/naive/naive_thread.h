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

#ifndef XDISPATCH_NAIVE_THREAD_H_
#define XDISPATCH_NAIVE_THREAD_H_

#include <thread>
#include <mutex>
#include <vector>

#include "backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class naive_thread : public ithread
    , public ithreadpool
{
public:
    naive_thread();

    ~naive_thread();

    void execute(
        const operation_ptr& work
    ) final;

    void execute(
        const operation_ptr& work,
        const queue_priority /* priority */
    ) final;

private:
    void drain();

    std::thread m_thread;
    std::mutex m_CS;
    std::vector< operation_ptr > m_ops;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_THREAD_H_ */
