/*
* thread_utils.cpp
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
#include "xdispatch/thread_utils.h"
#include "naive/naive_trace.h"

#include <stdio.h>

#if (defined XDISPATCH2_HAVE_PRCTL)
    #include <sys/prctl.h>
#endif

#if (defined XDISPATCH2_HAVE_SETPRIORITY)
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
    #include <sys/syscall.h>
#endif

#if (defined XDISPATCH2_HAVE_SYSCTL)
    #include <sys/sysctl.h>
#endif

#if (defined XDISPATCH2_HAVE_SYSCONF)
    #include <unistd.h>
#endif

__XDISPATCH_BEGIN_NAMESPACE

void thread_utils::set_current_thread_name(
    const std::string& name
)
{
#if (defined XDISPATCH2_HAVE_PTHREAD_SETNAME_NP)

    pthread_setname_np( name.c_str() );

#endif

#if (defined XDISPATCH2_HAVE_PRCTL)

    prctl( PR_SET_NAME, ( unsigned long )( name.c_str() ), 0, 0, 0 ); // NOLINT(runtime/int)

#endif
}

void thread_utils::set_current_thread_priority(
    queue_priority priority
)
{
#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)

    const qos_class_t qos_class = map_priority_to_qos( priority );
    pthread_set_qos_class_self_np( qos_class, 0 );

#ifdef DEBUG
    qos_class_t qos_actual = QOS_CLASS_DEFAULT;
    pthread_get_qos_class_np( pthread_self(), &qos_actual, nullptr );
    XDISPATCH_ASSERT( qos_actual == qos_class );
#endif

#elif (defined XDISPATCH2_HAVE_SETPRIORITY)

    int nice = 0;
    switch( priority )
    {
    case queue_priority::USER_INTERACTIVE:
        nice = 5;
        break;
    case queue_priority::USER_INITIATED:
        nice = 4;
        break;
    case queue_priority::UTILITY:
        nice = 3;
        break;
    case queue_priority::DEFAULT:
        nice = 0;
        break;
    case queue_priority::BACKGROUND:
        nice = 0;
        break;
    }
    const int tid = static_cast<int>( syscall( SYS_gettid ) );
    setpriority( PRIO_PROCESS, tid, nice );

#endif
}

#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)

qos_class_t thread_utils::map_priority_to_qos(
    queue_priority priority
)
{
    qos_class_t qos_class = QOS_CLASS_DEFAULT;
    switch( priority )
    {
    case queue_priority::USER_INTERACTIVE:
        qos_class = QOS_CLASS_USER_INTERACTIVE;
        break;
    case queue_priority::USER_INITIATED:
        qos_class = QOS_CLASS_USER_INITIATED;
        break;
    case queue_priority::UTILITY:
        qos_class = QOS_CLASS_UTILITY;
        break;
    case queue_priority::DEFAULT:
        qos_class = QOS_CLASS_DEFAULT;
        break;
    case queue_priority::BACKGROUND:
        qos_class = QOS_CLASS_BACKGROUND;
        break;
    }
    return qos_class;
}

#endif

size_t thread_utils::system_thread_count()
{
#if (defined XDISPATCH2_HAVE_SYSCONF) &&  (defined XDISPATCH2_HAVE_SYSCONF_SC_NPROCESSORS_ONLN)
    const auto nprocessors = sysconf( _SC_NPROCESSORS_ONLN );
    if( nprocessors < 0 )
    {
        XDISPATCH_TRACE() << "system_thread_count failed: " << strerror( errno ) << std::endl;
    }
    else
    {
        return nprocessors;
    }
#endif

#if (defined XDISPATCH2_HAVE_SYSCTL) && (defined XDISPATCH2_HAVE_SYSCTL_HW_NCPU)
    int value = 0;
    size_t length = sizeof( value );
    int mib [] = { CTL_HW, HW_NCPU };

    if( -1 == sysctl( mib, 2, &value, &length, NULL, 0 ) )
    {
        XDISPATCH_TRACE() << "system_thread_count failed: " << strerror( errno ) << std::endl;
    }
    else
    {
        return value;
    }
#endif

    // default
    return 4;
}


__XDISPATCH_END_NAMESPACE
