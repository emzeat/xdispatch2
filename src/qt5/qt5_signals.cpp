/*
 * qt5_signals.cpp
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

#include "qt5_signals.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5 {

void
register_connection(QObject* object, const connection& connection)
{
    auto* receiver_connection = ObjectConnectionManager::get(object, true);
    XDISPATCH_ASSERT(receiver_connection);
    receiver_connection->m_connections += connection;
}

void
destroy_connections(QObject* object, signal_p& signal)
{
    auto* receiver_connection = ObjectConnectionManager::get(object, false);
    if (receiver_connection) {
        receiver_connection->m_connections.reset_connections_with(signal);
    }
}

} // namespace qt5
__XDISPATCH_END_NAMESPACE
