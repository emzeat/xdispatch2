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

#ifndef XDISPATCH_NAIVE_CONSUMABLE_H_
#define XDISPATCH_NAIVE_CONSUMABLE_H_

#include "xdispatch/barrier_operation.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

class consumable;
using consumable_ptr = std::shared_ptr< consumable >;

class consumable
{
public:
    explicit consumable(
        size_t initialPayload = 0,
        const consumable_ptr& preceeding = consumable_ptr()
    );

    void increment(
        size_t by = 1
    );

    void consume();

    bool waitForConsumed(
        const std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    );

private:
    const consumable_ptr m_preceeding;
    std::atomic<size_t> m_payload;
    barrier_operation m_barrier;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_CONSUMABLE_H_ */
