/*
* signals.cpp
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

#include "xdispatch_internal.h"
#include "xdispatch/signals.h"

__XDISPATCH_BEGIN_NAMESPACE

connection_manager::~connection_manager()
{
    reset_connections();
}

void connection_manager::reset_connections()
{
    std::lock_guard<std::mutex> lock( m_CS );
    m_connections.clear();
}

connection_manager& connection_manager::operator +=(
    const connection& cn
)
{
    {
        std::lock_guard<std::mutex> lock( m_CS );
        m_connections.push_back( scoped_connection( cn ) );
    }
    return *this;
}

scoped_connection::scoped_connection(
    const connection& c
)
    : connection( c )
{

}

scoped_connection::scoped_connection()
    : connection( nullptr, nullptr )
{
    // a disconnected connection, e.g. for member variables
}

scoped_connection::scoped_connection(
    const scoped_connection& cOther
)
    : connection( cOther )
{
    scoped_connection& other = const_cast< scoped_connection& >( cOther );
    other.m_id = nullptr;
    other.m_parent = nullptr;
}

scoped_connection::~scoped_connection()
{
    connection::disconnect();
}

scoped_connection& scoped_connection::operator =(
    const connection& other
)
{
    connection::disconnect();
    connection::operator =( other );
    return *this;
}

scoped_connection& scoped_connection::operator =(
    const scoped_connection& cOther
)
{
    scoped_connection& other = const_cast< scoped_connection& >( cOther );
    this->m_id = other.m_id;
    this->m_parent = other.m_parent;
    other.m_id = nullptr;
    other.m_parent = nullptr;
    return *this;
}

connection::connection(
    const void* id,
    iconnectable* parent
)
    : m_id( id )
    , m_parent( parent )
{
    // nothing, an empty connection is fine
}

bool connection::disconnect()
{
    bool disconnected = false;
    if( m_parent )
    {
        disconnected = m_parent->disconnect( *this );
    }
    m_id = nullptr;
    m_parent = nullptr;
    return disconnected;
}

bool connection::connected() const
{
    return nullptr != m_id;
}

bool connection::operator ==(
    const connection& other
) const
{
    return m_id == other.m_id && m_parent == other.m_parent;
}

bool connection::operator !=(
    const connection& other
) const
{
    return m_id != other.m_id && m_parent != other.m_parent;
}

signal_p::job::job(
    const queue& q,
    notification_mode m
)
    : m_queue( q )
    , m_active( active_enabled )
    , m_pending( 0 ), m_mode( m )
{

}

void signal_p::job::disable()
{
    if( m_queue.is_current_queue() )
    {
        // recursion
        m_active.store( active_disabled );
    }
    else
    {
        int expected = active_enabled;
        do
        {
            expected = active_enabled;
            m_active.compare_exchange_strong( expected, active_disabled );
        }
        while( active_running == expected );
    }
}

void signal_p::job::enable()
{
    m_active.store( active_enabled );
}

bool signal_p::job::enter()
{
    int expected = active_enabled;
    return m_active.compare_exchange_strong( expected, active_running );
}

void signal_p::job::leave()
{
    int expected = active_running;
    m_active.compare_exchange_strong( expected, active_enabled );
}

signal_p::signal_p(
    const group& g
)
    : iconnectable()
    , m_group( g )
    , m_ids( 0 )
{
}

signal_p::~signal_p()
{
    std::lock_guard<std::mutex> lock( m_CS );

    for( auto it : m_jobs )
    {
        const job_ptr job = it.second;
        job->disable();
    }
    m_jobs.clear();
}

bool signal_p::disconnect(
    connection& c
)
{
    job_ptr job;
    {
        std::lock_guard<std::mutex> lock( m_CS );
        auto it = m_jobs.find( c.m_id );
        if( it != m_jobs.end() )
        {
            job = it->second;
            m_jobs.erase( it );
        }
    }
    c.m_id = nullptr;
    if( job )
    {
        job->disable();
        return true;
    }
    return false;
}

void signal_p::skip_all()
{
    std::lock_guard<std::mutex> lock( m_CS );

    for( auto it : m_jobs )
    {
        const job_ptr job = it.second;
        job->disable();
    }
}

connection signal_p::connect(
    const job_ptr& job
)
{
    void* id = nullptr;
    {
        std::lock_guard<std::mutex> lock( m_CS );
        do
        {
            id = reinterpret_cast<void*>( ++m_ids );
        }
        // catch duplicates and overflow
        while( nullptr == id || m_jobs.find( id ) != m_jobs.end() );

        m_jobs.insert( std::make_pair( id, job ) );
    }
    return connection( id, this );
}

__XDISPATCH_END_NAMESPACE
