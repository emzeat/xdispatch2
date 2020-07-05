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

#include <map>
#include <atomic>
#include <vector>
#include <mutex>
#include <functional>

#include "xdispatch/dispatch"

__XDISPATCH_BEGIN_NAMESPACE

class connection;

class iconnectable
{
public:
    iconnectable() = default;

    virtual ~iconnectable() = default;

    virtual bool disconnect(
        connection& c
    ) = 0;
};

class XDISPATCH_EXPORT connection
{
protected:
    connection(
        const void* id,
        iconnectable* parent
    );

public:
    bool disconnect();

    bool connected() const;

    bool operator ==(
        const connection& other
    ) const;

    bool operator !=(
        const connection& other
    ) const;

private:
    friend class iconnectable;
    friend class signal_p;
    friend class scoped_connection;

    const void* m_id;
    iconnectable* m_parent;
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
};

enum class notification_mode
{
    batch_updates,
    single_updates
};

class XDISPATCH_EXPORT signal_p : public iconnectable
{
protected:
    struct job;
    using job_ptr = std::shared_ptr< job > ;
    using job_map = std::map< const void*, job_ptr >;

public:
    signal_p(
        const group& g = group()
    );

    ~signal_p();

    XDISPATCH_WARN_UNUSED_RETURN( connection ) connect(
        const job_ptr& job
    );

    bool disconnect(
        connection& c
    ) final;

    void skip_all();

protected:
    struct job
    {
        enum
        {
            active_disabled = 0,
            active_enabled,
            active_running
        };

        job(
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
    job_map m_jobs;
    uintptr_t m_ids;
};

template<typename Signature>
class signal;

template<typename... Args>
class signal<void( Args... )> : public signal_p
{
public:
    typedef std::function< void( Args... ) > functor;

private:
    struct job_t : public job
    {
        job_t(
            const queue& q,
            const functor& f,
            notification_mode m
        )
            : job( q, m )
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
        job_ptr new_job = std::make_shared<job_t >( q, f, m );
        return signal_p::connect( new_job );
    }

    void operator()(
        Args... argList
    )
    {
        // FIXME(zwicker): Can this be pulled into the signal_p?
        std::lock_guard<std::mutex> lock( m_CS );

        for( auto it : m_jobs )
        {
            const job_ptr job = it.second;
            int pending = job->m_pending++;
            if( notification_mode::single_updates == job->m_mode || pending < 1 )
            {
                job->enable();
                m_group.async( [ = ]
                {
                    if( job->enter() )
                    {
                        job->m_pending--;
                        std::static_pointer_cast<job_t>( job )->m_func( argList... );
                        job->leave();
                    }
                }, job->m_queue );
            }
            else
            {
                job->m_pending--;
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
