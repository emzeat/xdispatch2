/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/

#ifndef XDISPATCH_TRACE_UTILS_H_
#define XDISPATCH_TRACE_UTILS_H_

#include <iostream>

#include "xdispatch_internal.h"
#include "xdispatch/ibackend.h"

__XDISPATCH_BEGIN_NAMESPACE

class trace_utils
{
public:
    static bool is_trace_enabled();
    static bool is_debug_enabled();
    static void assert_same_backend(
        backend_type a,
        backend_type b
    );

private:
    trace_utils() = delete;
};

#define XDISPATCH_TRACE_PREFIX "[xdispatch2] "

#define XDISPATCH_TRACE() \
    for( bool enabled = trace_utils::is_trace_enabled(); enabled; enabled = false ) \
        std::cout << XDISPATCH_TRACE_PREFIX

__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_TRACE_UTILS_H_ */
