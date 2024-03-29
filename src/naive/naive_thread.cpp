/*
 * naive_thread.cpp
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

#include "naive_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

thread::thread(const std::string& name, queue_priority priority)
  : manual_thread(name, priority)
  , m_thread(&manual_thread::run, this)
{}

thread::~thread()
{
    manual_thread::cancel();
    XDISPATCH_ASSERT(m_thread.joinable() && "Thread should not delete itself");
    m_thread.join();
}

} // namespace naive
__XDISPATCH_END_NAMESPACE
