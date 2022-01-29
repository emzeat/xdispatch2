/*
 * qt5_threadpool.h
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

#ifndef XDISPATCH_QT5_THREADPOOL_H_
#define XDISPATCH_QT5_THREADPOOL_H_

#include <QtCore/QThreadPool>
#include <QtCore/QPointer>

#include "qt5_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5 {

class ThreadPoolProxy : public naive::ithreadpool
{
public:
    ThreadPoolProxy(QThreadPool* pool);

    ~ThreadPoolProxy();

    void execute(const operation_ptr& work, queue_priority priority) final;

private:
    QPointer<QThreadPool> m_pool;
};

} // namespace qt5
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_QT5_THREADPOOL_H_ */
