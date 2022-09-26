/*
 * signals_barrier.h
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

#ifndef XDISPATCH_SIGNALS_BARRIER_H_
#define XDISPATCH_SIGNALS_BARRIER_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include <tuple>

#include "xdispatch/signals.h"
#include "xdispatch/impl/lightweight_barrier.h"

__XDISPATCH_BEGIN_NAMESPACE

template<typename Signature>
class signal_barrier;

/**
 * @brief Implements a barrier which may be used to synchronize
 *        with the next raise of a signal
 */
template<typename... Args>
class signal_barrier<void(Args...)>
{
    /// Expands to true if whole pack is true
    template<bool...>
    struct bool_pack;
    template<bool... bits>
    using all_true =
      std::is_same<bool_pack<bits..., true>, bool_pack<true, bits...>>;

    /// Expands to true if default constructible
    template<typename... Values>
    using all_default_constructible =
      all_true<std::is_default_constructible<Values>::value...>;

    /// Helper class to store a value, specialized to
    /// handle non default constructable types while
    /// still storing default constructable types efficiently
    template<int, bool = all_default_constructible<Args...>::value>
    struct value_holder
    {
        inline std::tuple<Args...> get() const { return m_values; }

        inline void set(Args... args) { m_values = std::make_tuple(args...); }

    private:
        std::tuple<Args...> m_values;
    };

    template<int unused>
    struct value_holder<unused, false>
    {
        inline std::tuple<Args...> get() const { return *m_values; }

        inline void set(Args... args)
        {
            m_values.reset(new std::tuple<Args...>(std::make_tuple(args...)));
        }

    private:
        std::unique_ptr<std::tuple<Args...>> m_values;
    };

public:
    /**
        @brief Constructs a new barrier

        @param signal The signal against which to synchronize
     */
    explicit signal_barrier(signal<void(Args...)>& signal)
      : m_signal(signal)
      , m_barrier()
      , m_connection(signal.connect([this](Args... values) {
          m_values.set(values...);
          m_connection.disconnect();
          m_barrier.complete();
      }))
    {}

    /**
        @brief Will wait for the signal to be raised

        If the signal has been raised since the barrier has been
        constructed, the call will return immediately

        @param timeout the maximum time to wait for the signal

        @returns true if the signal was raised or false if the timeout
                 expired before the signal
     */
    inline bool wait(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(-1))
    {
        return m_barrier.wait(timeout);
    }

    /**
        @return The values that had been emitted as part of the raise
                which caused the barrier to unblock.

        Further raises of the signal will leave the values unchanged.

        @throws std::runtime_error if called before the signal had been raised
     */
    inline std::tuple<Args...> values() const
    {
        if (m_barrier.was_completed()) {
            return m_values.get();
        }
        throw std::runtime_error("Barrier was not signalled yet");
    }

    /**
        @return The value that had been emitted as part of the raise
                which caused the barrier to unblock.

        Further raises of the signal will leave the value unchanged.

        @tparam index Use this to retrieve the passed value in case the
                      signal carries more than one argument

        @throws std::runtime_error if called before the signal had been raised
     */
    template<int index = 0>
    inline typename std::tuple_element<index, std::tuple<Args...>>::type value()
      const
    {
        return std::get<index>(values());
    }

private:
    signal<void(Args...)>& m_signal;
    lightweight_barrier m_barrier;
    scoped_connection m_connection;
    value_holder<0> m_values;
};

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_SIGNALS_BARRIER_H_ */
