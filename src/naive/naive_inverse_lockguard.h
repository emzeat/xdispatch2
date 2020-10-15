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

#ifndef XDISPATCH_NAIVE_INVERSE_LOCKGUARD_H_
#define XDISPATCH_NAIVE_INVERSE_LOCKGUARD_H_

#include <mutex>

#include "naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

/**
    @brief Inverse to std::lock_guard

    Will unlock on construction and relock
    the mutex on destruction
 */
template <class _Mutex>
class inverse_lock_guard
{
public:
    inverse_lock_guard
    (
        _Mutex& cs
    )
        : m_CS( cs )
    {
        m_CS.unlock();
    }

    ~inverse_lock_guard()
    {
        m_CS.lock();
    }

private:
    _Mutex& m_CS;
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_INVERSE_LOCKGUARD_H_ */
