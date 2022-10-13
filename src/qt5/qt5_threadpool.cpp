/*
 * qt5_threadpool.cpp
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

#include <QtCore/QRunnable>

#include "qt5_threadpool.h"

#include "../thread_utils.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5 {

ThreadPoolProxy::ThreadPoolProxy(QThreadPool* pool)
  : m_pool(pool)
{
    XDISPATCH_ASSERT(pool);
}

ThreadPoolProxy::~ThreadPoolProxy()
{
    if (m_pool) {
        m_pool->waitForDone();
    }
}

void
ThreadPoolProxy::execute(const operation_ptr& work,
                         const queue_priority priority)
{
    class OperationRunnable : public QRunnable
    {
    public:
        OperationRunnable(const operation_ptr& op)
          : m_operation(op)
        {
            setAutoDelete(true);
        }

        void run() final { execute_operation_on_this_thread(*m_operation); }

    private:
        const operation_ptr m_operation;
    };

    int p = 0;
    switch (priority) {
        case queue_priority::BACKGROUND:
            p = 0; // NOLINT(readability-magic-numbers)
            break;
        case queue_priority::DEFAULT:
        case queue_priority::UTILITY:
            p = 20; // NOLINT(readability-magic-numbers)
            break;
        case queue_priority::USER_INITIATED:
            p = 40; // NOLINT(readability-magic-numbers)
            break;
        case queue_priority::USER_INTERACTIVE:
            p = 100; // NOLINT(readability-magic-numbers)
            break;
    }

    XDISPATCH_ASSERT(m_pool);
    m_pool->start(new OperationRunnable(work), p);
}

void
ThreadPoolProxy::notify_thread_blocked()
{
    m_pool->reserveThread();
}

void
ThreadPoolProxy::notify_thread_unblocked()
{
    m_pool->releaseThread();
}

} // namespace qt5
__XDISPATCH_END_NAMESPACE
