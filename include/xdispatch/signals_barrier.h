/*
* signals_barrier.h
*
* Copyright (c) 2011-2018 MLBA-Team
* All rights reserved.
*
* @LICENSE_HEADER_START@
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
* @LICENSE_HEADER_END@
*/

#ifndef XDISPATCH_SIGNALS_BARRIER_H_
#define XDISPATCH_SIGNALS_BARRIER_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include <tuple>

#include "xdispatch/signals.h"
#include "xdispatch/barrier_operation.h"

__XDISPATCH_BEGIN_NAMESPACE

template<typename Signature>
class signal_barrier;

template<typename... Args>
class signal_barrier<void( Args... )>
{
public:
    explicit signal_barrier(
        signal< void( Args... ) >& signal
    )
        : m_signal( signal )
        , m_barrier()
        , m_connection( signal.connect( [this]( Args... values )
    {
        m_values = std::make_tuple( values... );
        m_barrier();
    } ) )
    {
    }

    inline void wait(
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    )
    {
        m_barrier.wait( timeout );
    }

    inline std::tuple< Args ... > values() const
    {
        return m_values;
    }

private:
    signal< void( Args... ) >& m_signal;
    barrier_operation m_barrier;
    scoped_connection m_connection;
    std::tuple< Args ... > m_values;
};

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_SIGNALS_BARRIER_H_ */
