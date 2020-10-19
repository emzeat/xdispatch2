/*
* signals.h
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

#ifndef XDISPATCH_SIGNALS_H_
#define XDISPATCH_SIGNALS_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include <atomic>
#include <vector>
#include <mutex>
#include <functional>

#include "xdispatch/dispatch"

__XDISPATCH_BEGIN_NAMESPACE

class signal_p;

class XDISPATCH_EXPORT connection
{
public:
    connection();

    bool disconnect();

    bool connected() const;

    bool operator ==(
        const connection& other
    ) const;

    bool operator !=(
        const connection& other
    ) const;

protected:
    connection(
        const std::shared_ptr<void>& id,
        signal_p* parent
    );

private:
    friend class signal_p;
    friend class scoped_connection;

    std::weak_ptr<void> m_id;
    signal_p* m_parent;
};

class XDISPATCH_EXPORT scoped_connection : public connection
{
public:
    explicit scoped_connection(
        const connection& c
    );

    scoped_connection();

    scoped_connection(
        const scoped_connection& cOther
    );

    ~scoped_connection();

    scoped_connection& operator =(
        const connection& other );

    scoped_connection& operator =(
        const scoped_connection& cOther
    );

    connection take();
};

enum class notification_mode
{
    batch_updates,
    single_updates
};

class XDISPATCH_EXPORT signal_p
{
protected:
    class connection_handler;
    using connection_handler_ptr = std::shared_ptr< connection_handler > ;

public:
    signal_p(
        const group& g = group()
    );

    ~signal_p();

    bool disconnect(
        connection& c
    );

    void skip_all();

protected:
    connection connect(
        const connection_handler_ptr& job
    );

    class connection_handler
    {
    public:
        enum
        {
            active_disabled = 0,
            active_enabled,
            active_running
        };

        connection_handler(
            const queue& q,
            notification_mode m
        );

        void disable();

        void enable();

        bool enter();

        void leave();

        const queue m_queue;
        std::atomic<int> m_active;
        std::atomic<int> m_pending;
        notification_mode m_mode;
    };

    std::mutex m_CS;
    group m_group;
    std::vector< connection_handler_ptr > m_handlers;
};

template<typename Signature>
class signal;

template<typename... Args>
class signal<void( Args... )> : public signal_p
{
public:
    typedef std::function< void( Args... ) > functor;

private:
    class connection_handler_t : public connection_handler
    {
    public:
        connection_handler_t(
            const queue& q,
            const functor& f,
            notification_mode m
        )
            : connection_handler( q, m )
            , m_func( f )
        {
        }

        const functor m_func;
    };

public:
    signal(
        const group& g = group()
    )
        : signal_p( g )
    {
    }

    XDISPATCH_WARN_UNUSED_RETURN( connection ) connect(
        const functor& f,
        queue q = global_queue(),
        notification_mode m = notification_mode::single_updates
    )
    {
        connection_handler_ptr new_connection = std::make_shared<connection_handler_t>( q, f, m );
        return signal_p::connect( new_connection );
    }

    template<class T, typename... FunctionArgs>
    XDISPATCH_WARN_UNUSED_RETURN( connection ) connect(
        T* object,
        void( T::*function )( FunctionArgs... ),
        queue q = global_queue(),
        notification_mode m = notification_mode::single_updates
    )
    {
        return connect( [object, function]( Args... args )
        {
            ( object->*function )( args... );
        }, q, m );
    }

    void operator()(
        Args... argList
    )
    {
        // FIXME(zwicker): Can this be pulled into the signal_p?
        std::lock_guard<std::mutex> lock( m_CS );

        for( const connection_handler_ptr& handler : m_handlers )
        {
            int pending = handler->m_pending++;
            if( notification_mode::single_updates == handler->m_mode || pending < 1 )
            {
                handler->enable();
                m_group.async( [ = ]
                {
                    if( handler->enter() )
                    {
                        handler->m_pending--;
                        std::static_pointer_cast<connection_handler_t>( handler )->m_func( argList... );
                        handler->leave();
                    }
                }, handler->m_queue );
            }
            else
            {
                handler->m_pending--;
            }
        }
    }
};

class XDISPATCH_EXPORT connection_manager
{
public:
    connection_manager() = default;

    ~connection_manager();

    void reset_connections();

    connection_manager& operator +=(
        const connection& cn
    );

private:
    connection_manager(
        const connection_manager&
    ) = delete;

    std::mutex m_CS;
    std::vector< scoped_connection > m_connections;
};

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_SIGNALS_H_ */
