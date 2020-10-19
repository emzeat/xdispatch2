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

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

void register_connection(
    QObject* object,
    const connection& connection
)
{
    class object_connection : public QObject
    {
    public:
        connection_manager m_connections;
    };

    auto receiver_connection = object->findChild<object_connection*>( QString(), Qt::FindDirectChildrenOnly );
    if( nullptr == receiver_connection )
    {
        receiver_connection = new object_connection;
        receiver_connection->setParent( object );
    }
    XDISPATCH_ASSERT( receiver_connection );
    receiver_connection->m_connections += connection;
}

}
__XDISPATCH_END_NAMESPACE
