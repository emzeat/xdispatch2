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

/**
    @brief Describes a connection between a signal and a connected handler

    Can be obtained upon making the connection and be used to destroy
    the connection again later, making the handler and the signal independent again.
 */
class XDISPATCH_EXPORT connection
{
public:
    /**
        @brief Default constructor
     */
    connection();

    /**
        @brief Destroys the connection between signal and handler

        @see signal::disconnect
     */
    bool disconnect();

    /**
        @brief Indicates if this object describes an active connection

        @returns false for default constructed or already disconnected objects
     */
    bool connected() const;

    /**
        @returns true when the connection is to signal
     */
    bool is_connected_to(
        const signal_p&
    ) const;

    /**
        @brief Compares this connection to another

        @returns true if both the connected signals and handlers are identical
     */
    bool operator ==(
        const connection& other
    ) const;

    /**
        @brief Compares this connection to another

        @returns true if neither the connected signals nor handlers are identical
     */
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

/**
    @brief Helper to auto manage the lifetime of a connection

    This is similar in use to a std::unique_ptr
 */
class XDISPATCH_EXPORT scoped_connection : public connection
{
public:
    /**
        @brief Creates a new connection scope

        @param c The connection that will be managed
     */
    explicit scoped_connection(
        const connection& c
    );

    /**
        @brief Creates a new empty connection scope

        NO connection will be managed
     */
    scoped_connection();

    /**
        @brief Move constructor

        All connection ownership will be transferred from other
     */
    scoped_connection(
        scoped_connection&& other
    ) noexcept;

    /**
        @brief Destructor

        If holding a connection it will be disconnected automatically
     */
    ~scoped_connection();

    /**
        @brief Assigns a new connection to be managed

        @param c The connection that will be managed

        Any previously managed connection will be disconnected first
        before starting to manage the new connection.
     */
    scoped_connection& operator =(
        const connection& c
    );

    /**
        @brief Move assignment operator

        All connection ownership will be transferred from other
     */
    scoped_connection& operator =(
        scoped_connection&& other
    ) noexcept;

    /**
        @brief Releases the managed connection

        The responsibility for managing the contained connection
        will be up to the caller.
     */
    connection take();
};

/**
    @brief Describes the different notification modes supported
           when connecting to a signal
 */
enum class notification_mode
{
    //!< When a signal gets emitted a second time before the handler for the first
    //! emit was executed on the target queue, the second emit will not result in
    //! an additional handler call. Use this to connect to high frequency signals
    //! without overloading the receiving queue.
    batch_updates,
    //!< Each emit of a signal will result in the handler to be invoked.
    //! Use this if indiviual signal values or each signal emit are important
    //! and must never be missed
    single_updates
};

/**
    @private base used to implement signal
 */
class XDISPATCH_EXPORT signal_p
{
protected:
    class connection_handler;
    using connection_handler_ptr = std::shared_ptr< connection_handler > ;

public:
    /// Creates a new signal_p queuing all handler invocations into group
    signal_p(
        const group& g
    );

    /// Destructor
    ~signal_p();

    /**
        @brief Desctroys the givne connection

        @param c The connection to destroy, i.e. disconncet

        @return true if the connection was valid and there was something to disconnect
     */
    bool disconnect(
        connection& c
    );

    /**
        @brief Helper to supress all queued handler invocations

        IF a handler is currently active, its call will complete but no
        other queued calls will be made. When the signal gets emitted for
        the next time, new calls will be queued.
     */
    void skip_all();

protected:
    /// Registers a connection for the given handler
    connection connect(
        const connection_handler_ptr& job
    );

    /// Disallow copying of a signal to ensure consistent behavior
    signal_p(
        const signal_p&
    ) = delete;

    /// Private container class for holding a handler
    class connection_handler
    {
    public:
        enum active_state
        {
            /// Handler is not in a queue right now
            active_disabled = 0,
            /// Handler has been queued but not executed yet
            active_enabled,
            /// Handler is actively being executed
            active_running
        };

        /// Creates a handler for the given queue and notification mode
        connection_handler(
            const queue& q,
            notification_mode m
        );

        /// Marks the handler as not to be queued anywhere,
        /// i.e. sets it to active_disabled
        void disable();

        /// Marks the handler as having been queued,
        /// i.e. sets it to active_enabled
        void enable();

        /// Notifies the handler that it is about to be called,
        /// i.e. sets it to active_running if the handler had been enabled
        /// @returns true if the handler had been active_enabled
        bool enter();

        /// Notifies the handler that the call is done,
        /// i.e. sets it to active_enabled again if it had
        /// been in the active_running state before
        void leave();

        // the queue to execute the handler on
        const queue m_queue;
        // the current state of the handler
        std::atomic<active_state> m_active;
        // the number of items that have been queued
        std::atomic<int> m_pending;
        // the mode in which the handler will be called
        notification_mode m_mode;
    };

    // Protects parallel access
    std::mutex m_CS;
    // The group used to track all handler calls
    group m_group;
    // List of connected handlers
    std::vector< connection_handler_ptr > m_handlers;
};

template<typename Signature>
class signal;

/**
    @brief Defines a signal transporting arbitrary values

    Signals can be used to have a producer emit certain updates
    to which consumers can connect their own handlers while making
    sure that these handlers get invoked on the correct queues.

    @tparam Args The signature that handlers connecting to the
                 signal need to follow. This also defines the data
                 carried with each signal emit.

    @remark The template parameter is kept compatible with boost::signal
            however due to the asynchronous handling of signal handlers,
            only handlers returning void are supported.
 */
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
    /**
        @brief Constructs a new signal instance

        @param g An optional group to send all signal emits through
                 This can be used if the signal owner needs to synchronize
                 with active handler invocations
     */
    signal(
        const group& g = group()
    )
        : signal_p( g )
    {
    }

    /**
        @brief Adds a new connection to the signal

        @param f The functor to invoke when the signal fires, this needs
                 to be an object which is callable
        @param q The queue on which the functor will be invoked
        @param m The mode to use for the new connection
     */
    XDISPATCH_WARN_UNUSED_RETURN( connection ) connect(
        const functor& f,
        const queue& q = global_queue(),
        notification_mode m = notification_mode::single_updates
    )
    {
        connection_handler_ptr new_connection = std::make_shared<connection_handler_t>( q, f, m );
        return signal_p::connect( new_connection );
    }

    /**
        @brief Adds a new connection to the signal

        @param object The object to notify when the signal fires
        @param function The member function to invoke on object
        @param q The queue on which the member function will be invoked
        @param m The mode to use for the new connection
     */
    template<class T, typename... FunctionArgs>
    XDISPATCH_WARN_UNUSED_RETURN( connection ) connect(
        T* object,
        void( T::*function )( FunctionArgs... ),
        const queue& q = global_queue(),
        notification_mode m = notification_mode::single_updates
    )
    {
        return connect( [object, function]( Args... args )
        {
            ( object->*function )( args... );
        }, q, m );
    }

    /**
        @param Emits the signal notifying all active connections
     */
    void operator()(
        Args... argList
    )
    {
        // FIXME(zwicker): Can this be pulled into the signal_p?
        std::lock_guard<std::mutex> lock( m_CS );

        for( const connection_handler_ptr& handler : m_handlers )
        {
            auto pending = handler->m_pending++;
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

/**
    @brief Utility class to manage a list of connections
 */
class XDISPATCH_EXPORT connection_manager
{
public:
    /**
        @brief Default constructor
     */
    connection_manager() = default;

    /**
        @brief Destructor
     */
    ~connection_manager();

    /**
        @brief Destroys all connections currently managed
     */
    void reset_connections();

    /**
        @brief Destroys all managed connections to signal
     */
    void reset_connections_with(
        const signal_p& s
    );

    /**
        @brief Adds the given connection to the manager

        It will be automatically destroyed when the manager goes out of scope
     */
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
