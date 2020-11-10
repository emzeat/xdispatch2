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

#include "qt5_backend_internal.h"

#include "xdispatch/iqueue_impl.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

itimer_impl_ptr backend::create_timer(
    const iqueue_impl_ptr& queue
)
{
    if( backend_type::qt5 == queue->backend() )
    {
        return naive::backend::create_timer( queue, backend_type::qt5 );
    }
    else
    {
        return backend_base::create_timer( queue );
    }
}

}
__XDISPATCH_END_NAMESPACE
