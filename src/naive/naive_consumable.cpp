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

#include "naive_consumable.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

consumable::consumable(
    size_t initialPayload,
    const consumable_ptr& preceeding
)
    : m_preceeding( preceeding )
    , m_payload( initialPayload )
{
}

void consumable::increment(
    size_t by
)
{
    m_payload.fetch_add( by );
}

void consumable::consume()
{
    if( 1 == m_payload.fetch_sub( 1 ) )
    {
        m_barrier();
    }
}

bool consumable::waitForConsumed(
    const std::chrono::milliseconds timeout
)
{
    if( m_preceeding )
    {
        if( m_preceeding->waitForConsumed( timeout ) )
        {
            // satisfied, continue with this
        }
        else
        {
            return false;
        }
    }
    if( 0 == m_payload.load() )
    {
        return true;
    }
    return m_barrier.wait( timeout );
}

}
__XDISPATCH_END_NAMESPACE
