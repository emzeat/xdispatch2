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

class object_connection : public QObject
{
public:
    connection_manager m_connections;

    static object_connection* get(
        QObject* object,
        bool createIfMissing = false
    )
    {
        auto oc = object->findChild<object_connection*>( QString(), Qt::FindDirectChildrenOnly );
        if( createIfMissing && nullptr == oc )
        {
            oc = new object_connection;
            oc->setParent( object );
        }
        return oc;
    }
};

void register_connection(
    QObject* object,
    const connection& connection
)
{
    auto receiver_connection = object_connection::get( object, true );
    XDISPATCH_ASSERT( receiver_connection );
    receiver_connection->m_connections += connection;
}

void destroy_connections(
    QObject* object,
    signal_p& signal
)
{
    auto receiver_connection = object_connection::get( object, false );
    if( receiver_connection )
    {
        receiver_connection->m_connections.reset_connections_with( signal );
    }
}

}
__XDISPATCH_END_NAMESPACE
