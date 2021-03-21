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

#include "naive_manual_thread.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive {

/**
    An implementation of ithread executing on its own thread
 */
class thread : public manual_thread
{
public:
    /**
        @param name The name by which the thread is known
        @param priority The default priority for which the thread will execute
     */
    thread(const std::string& name, queue_priority priority);

    /**
        @brief Destructor
     */
    ~thread() override;

private:
    std::thread m_thread;
};

} // namespace naive
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_THREAD_H_ */
