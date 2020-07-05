/*
* thread_utils.h
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

#ifndef XDISPATCH_THREAD_UTILS_H_
#define XDISPATCH_THREAD_UTILS_H_

#include "xdispatch/dispatch.h"

#if (defined __APPLE__)
    #include <sys/qos.h>
#endif

__XDISPATCH_BEGIN_NAMESPACE

/**
    @brief Utilities to help with development using xdispatch
 */
class thread_utils
{
public:
    /**
        @brief Sets the name of the current thread

        The assigned name will show up in the debugger and can be used
        as an aid to identify execution paths. By default this will be
        used to assign names according to the currently executing queue
     */
    static void set_current_thread_name(
        const std::string& name
    );

    /**
        @brief Sets the priority of the current thread

        The assigned priority will take effect immediately
     */
    static void set_current_thread_priority(
        queue_priority priority
    );

#if (defined __APPLE__)
    /**
        @returns the qos class mapped to the given queue_priority
    */
    static qos_class_t map_priority_to_qos(
        queue_priority priority
    );
#endif

private:
    thread_utils() = delete;
};

__XDISPATCH_END_NAMESPACE


#endif // XDISPATCH_THREAD_UTILS_H_
