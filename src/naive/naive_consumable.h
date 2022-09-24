/*
 * naive_consumable.h
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

#ifndef XDISPATCH_NAIVE_CONSUMABLE_H_
#define XDISPATCH_NAIVE_CONSUMABLE_H_

#include "xdispatch/impl/lightweight_barrier.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

class consumable;
using consumable_ptr = std::shared_ptr<consumable>;

/**
    @brief Manages a list of resources which can be consumed

    This is similar to a counting semaphore but in a backward
    way in that a caller will get unblocked when no resources
    are left, not when a resource is left as with a semaphore.

    Consumables can be daisy-chained to model a dependent list
    of resources that need to be drained up before proceeding.
 */
class consumable
{
public:
    /**
        @brief Creates a new consumable

        @param resources The initial number of resources
                    that can be consumed from the very beginning
        @param preceeding A preceeding consumable that needs to be
                    freed up as well before this consumable
                    is considered as fully consumed
     */
    explicit consumable(size_t resources = 0,
                        const consumable_ptr& preceeding = consumable_ptr());

    /**
        @brief Adds an additional free resource to the consumable

        If the consumable had already been fully consumed by the
        time this is called, this will have no effect and the
        consumable remain fully consumed.
     */
    void add_resource();

    /**
        @brief Consumes a resource removing it from the list of
               available resources
     */
    void consume_resource();

    /**
        @brief Blocks until all resources to be consumed or the
               given timeout has passed whatever comes first

        @param timeout The maximum time to wait for resources to be used

        @return true if all resources have been consumed before the
                timeout has elapsed
     */
    bool wait_for_consumed(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(-1));

private:
    const consumable_ptr m_preceeding;
    std::atomic<size_t> m_resources;
    lightweight_barrier m_barrier;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_CONSUMABLE_H_ */
