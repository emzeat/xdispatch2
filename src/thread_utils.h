/*
 * thread_utils.h
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

#ifndef XDISPATCH_THREAD_UTILS_H_
#define XDISPATCH_THREAD_UTILS_H_

#include "xdispatch/dispatch.h"

#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)
    #include <pthread.h>
    #include <sys/qos.h>
#endif

__XDISPATCH_BEGIN_NAMESPACE

/**
    @brief Utilities to help with development using xdispatch
 */
class XDISPATCH_EXPORT thread_utils
{
public:
    /**
        @brief Sets the name of the current thread

        The assigned name will show up in the debugger and can be used
        as an aid to identify execution paths. By default this will be
        used to assign names according to the currently executing queue
     */
    static void set_current_thread_name(const std::string& name);

    /**
        @brief Sets the priority of the current thread

        The assigned priority will take effect immediately
     */
    static void set_current_thread_priority(queue_priority priority);

#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)
    /**
        @returns the queue_priority mapped to the related qos class
    */
    static qos_class_t map_priority_to_qos(queue_priority priority);
#endif

#if (defined XDISPATCH2_HAVE_SETPRIORITY)
    /**
        @returns the queue_priority mapped to a matching nice level chosen
                 relative to the nice level the process was originally started
       with.
    */
    static int map_priority_to_nice(queue_priority priority);
#endif

    /**
        @brief Gets the ideal number of threads for running on this system

        The number is determined by querying the available cores (both
        physical and logical).

        If this information cannot detemrined, a default of 4 will be used.
     */
    static size_t system_thread_count();

    /**
        @brief CPU relax instruction

        This is meant to optimize access to a resource blocked by another
        CPU but known to become available within a few instructions
     */
    static void cpu_relax();

private:
    thread_utils() = delete;
};

__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_THREAD_UTILS_H_
