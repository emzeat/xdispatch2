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

#ifndef XDISPATCH_QT5_SIGNALS_H_
#define XDISPATCH_QT5_SIGNALS_H_

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QObject>

#include "qt5_backend_internal.h"

__XDISPATCH_BEGIN_NAMESPACE
namespace qt5
{

/**
   Manages connections between a QObject
   and one or more xdispatch signals

   @see qt5::connect
*/
class ObjectConnectionManager : public QObject
{
    Q_OBJECT

public:
    connection_manager m_connections;

    /**
       @returns an ObjectConnectionManager associated with the object
                or null if no such manager exists and also should not get
                created

       @param object The object to associate the manager with
       @param createIfMissing pass true to create a new manager if none
                     exists yet
     */
    static ObjectConnectionManager* get(
        QObject* object,
        bool createIfMissing = false
    )
    {
        static QMutex sCS; // shared lock to ensure no two managers get created for an object

        QMutexLocker lock( &sCS );
        auto oc = object->findChild<ObjectConnectionManager*>( QString(), Qt::FindDirectChildrenOnly );
        if( createIfMissing && nullptr == oc )
        {
            oc = new ObjectConnectionManager;
            oc->setParent( object );
        }
        return oc;
    }
};

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_QT5_SIGNALS_H_ */
