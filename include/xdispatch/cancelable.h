/*
 * cancelable.h
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

#ifndef XDISPATCH_CANCELABLE_H_
#define XDISPATCH_CANCELABLE_H_

#include <atomic>

/**
 * @addtogroup xdispatch
 * @{
 */

#include "xdispatch/dispatch.h"

__XDISPATCH_BEGIN_NAMESPACE

/**
    @brief Describes an entity which can be cancelled

    Whenever the entity becomes active it should enter().
    If not cancelled this will return true in which case the
    entity can do its processing. Once done it must invoke
    leave() to indicate it is not active anymore.

    Use disable() to cancel the entity.
*/
class XDISPATCH_EXPORT cancelable
{
public:
    enum active_state
    {
        /**< Entity is not in a queue right now */
        active_disabled = 0,
        /**< Entity has been queued but not executed yet */
        active_enabled,
        /**< Entity is actively being executed */
        active_running
    };

    /**
       @brief Constructs an active (not disabled) cancelable.
     */
    cancelable();

    /**
       Marks the entity as not to be queued anywhere,
       i.e. sets it to active_disabled
       @param executor_queue The queue on which the entity would be executed
    */
    void disable(const queue& executor_queue);

    /**
       Marks the entity as not to be queued anywhere,
       i.e. sets it to active_disabled
       @param executor_queue The queue on which the entity would be executed
    */
    void disable(const iqueue_impl_ptr& executor_queue);

    /**
        Marks the entity as having been queued,
        i.e. sets it to active_enabled
    */
    void enable();

    /**
        Notifies the entity that it is about to be called,
        i.e. sets it to active_running if the entity had been enabled

        Make sure to balance this with calls to leave().
        It is recommended to not invoket his directly but go through
        cancelable_scope instead which manages this for you.

        @returns true if the entity had been active_enabled
    */
    bool enter();

    /**
        Notifies the entity that the call is done,
        i.e. sets it to active_enabled again if it had
        been in the active_running state before
    */
    void leave();

private:
    // the current state of the handler
    std::atomic<active_state> m_active;
};

/**
   @brief RAII handling of enter() / leave() for a cancelable
 */
class cancelable_scope
{
public:
    /**
       @brief Enters the cancelable
     */
    inline explicit cancelable_scope(cancelable& c)
      : m_c(c)
      , m_active(m_c.enter())
    {}

    /**
       @brief Leaves the cancelable
     */
    inline ~cancelable_scope()
    {
        if (m_active) {
            m_c.leave();
        }
    }

    /**
       @returns true if the cancelable was not disabled when entering it
     */
    inline operator bool() const { return m_active; }

private:
    cancelable& m_c;
    const bool m_active;
};

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_CANCELABLE_H_ */
