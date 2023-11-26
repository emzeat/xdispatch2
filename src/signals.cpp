/*
 * signals.cpp
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
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

#include <algorithm>

#include "xdispatch_internal.h"
#include "xdispatch/signals.h"

__XDISPATCH_BEGIN_NAMESPACE

connection_manager::~connection_manager()
{
    reset_connections();
}

void
connection_manager::reset_connections()
{
    std::lock_guard<std::mutex> lock(m_CS);
    m_connections.clear();
}

void
connection_manager::reset_connections_with(const signal_p& s)
{
    std::lock_guard<std::mutex> lock(m_CS);
    const auto last = std::remove_if(
      m_connections.begin(),
      m_connections.end(),
      [&s](const scoped_connection& sc) { return sc.is_connected_to(s); });
    m_connections.erase(last, m_connections.end());
}

connection_manager&
connection_manager::operator+=(const connection& cn)
{
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_connections.emplace_back(cn);
    }
    return *this;
}

scoped_connection::scoped_connection(const connection& c)
  : connection(c)
{}

scoped_connection::scoped_connection()
  : connection(nullptr, nullptr)
{
    // a disconnected connection, e.g. for member variables
}

scoped_connection::scoped_connection(scoped_connection&& other) noexcept
  : connection(std::move(other))
{
    // NOLINTBEGIN(bugprone-use-after-move)
    other.m_id.reset();
    other.m_parent = nullptr;
    // NOLINTEND(bugprone-use-after-move)
}

scoped_connection::~scoped_connection()
{
    connection::disconnect();
}

scoped_connection&
scoped_connection::operator=(const connection& other)
{
    connection::disconnect();
    connection::operator=(other);
    return *this;
}

scoped_connection&
scoped_connection::operator=(scoped_connection&& other) noexcept
{
    connection::disconnect();
    connection::operator=(other);
    other.m_id.reset();
    other.m_parent = nullptr;
    return *this;
}

connection
scoped_connection::take()
{
    connection other;
    other.m_id = m_id;
    other.m_parent = m_parent;
    m_id.reset();
    m_parent = nullptr;
    return other;
}

connection::connection(const std::shared_ptr<void>& id, signal_p* parent)
  : m_id(id)
  , m_parent(parent)
{
    // nothing, an empty connection is fine
}

connection::connection()
  : connection(nullptr, nullptr)
{}

bool
connection::disconnect()
{
    bool disconnected = false;
    if (m_parent) {
        disconnected = m_parent->disconnect(*this);
    }
    m_id.reset();
    m_parent = nullptr;
    return disconnected;
}

bool
connection::connected() const
{
    return !m_id.expired();
}

bool
connection::is_connected_to(const signal_p& signal) const
{
    const auto* const signal_ptr = &signal;
    return signal_ptr == m_parent;
}

bool
connection::operator==(const connection& other) const
{
    const auto id = m_id.lock();
    const auto other_id = other.m_id.lock();
    return id == other_id && m_parent == other.m_parent;
}

bool
connection::operator!=(const connection& other) const
{
    return !(other == *this);
}

signal_p::connection_handler::connection_handler(const queue& q,
                                                 notification_mode m)
  : m_queue(q)
  , m_active()
  , m_pending(0)
  , m_mode(m)
{}

void
signal_p::connection_handler::disable()
{
    m_active.disable();
}

signal_p::signal_p(const group& g)
  : m_group(new group(g))
{}

signal_p::signal_p()
  : m_group()
{}

signal_p::~signal_p()
{
    std::lock_guard<std::mutex> lock(m_CS);

    for (const auto& handler : m_handlers) {
        handler->disable();
    }
    m_handlers.clear();
}

bool
signal_p::disconnect(connection& c)
{
    const auto c_void = c.m_id.lock();
    const auto c_handler = std::static_pointer_cast<connection_handler>(c_void);
    if (c_handler) {
        std::lock_guard<std::mutex> lock(m_CS);
        const auto last =
          std::remove(m_handlers.begin(), m_handlers.end(), c_handler);
        m_handlers.erase(last, m_handlers.end());
    }
    c.m_id.reset();
    if (c_handler) {
        c_handler->disable();
        return true;
    }
    return false;
}

void
signal_p::skip_all()
{
    std::lock_guard<std::mutex> lock(m_CS);

    for (const auto& handler : m_handlers) {
        handler->disable();
    }
}

connection
signal_p::connect(const connection_handler_ptr& job)
{
    {
        std::lock_guard<std::mutex> lock(m_CS);
        m_handlers.push_back(job);
    }
    return connection(job, this);
}

__XDISPATCH_END_NAMESPACE
