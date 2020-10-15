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
#include "naive_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

consumable::consumable(
    size_t resources,
    const consumable_ptr& preceeding
)
    : m_preceeding( preceeding )
    , m_resources( resources )
{
}

void consumable::add_resource()
{
    m_resources.fetch_add( size_t( 1 ) );
}

void consumable::consume_resource()
{
    // brief test to verify resources will never get negative
    XDISPATCH_ASSERT( m_resources.load() > 0 );
    // when we consume the last resource, i.e.
    // the count is one at the time of this call
    // we can complete the barrier
    if( 1 == m_resources.fetch_sub( size_t( 1 ) ) )
    {
        m_barrier();
    }
}

bool consumable::wait_for_consumed(
    const std::chrono::milliseconds timeout
)
{
    // make sure that the preceeding consumable is satisfied
    if( m_preceeding )
    {
        if( m_preceeding->wait_for_consumed( timeout ) )
        {
            // satisfied, continue with this
        }
        else
        {
            return false;
        }
    }
    // if no resources have been available or consumed
    // the barrier will never be completed but the consumable
    // can be considered as fully satisfied still
    if( 0 == m_resources.load() )
    {
        return true;
    }
    return m_barrier.wait( timeout );
}

}
__XDISPATCH_END_NAMESPACE
