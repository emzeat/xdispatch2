/*
* main.c
*
* Copyright (c) 2011-2013 MLBA-Team.
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

#include "munit/MUnit.h"
#include "cxx_tests.h"
#include "platform_tests.h"
#include "signal_tests.h"

#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    #include "../src/libdispatch/libdispatch_backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    #include "../src/naive/naive_backend_internal.h"
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    #include <QtCore/QCoreApplication>
    #include "../src/qt5/qt5_backend_internal.h"
#endif

void print_log(
    const char* msg
)
{
    printf( "%s", msg );
    fflush( stdout );
}

/*
 The test program for testing the xdispatch2
 framework. The difficult task in here is
 that all functions dispatched can be started
 at any time. Thus each test has to run as own
 process, end when all functions were executed,
 and be supervised by the test application
 */

int main( int argc, char* argv[] )
{
    int ret = 0;
    MU_initFramework( print_log );

#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    static xdispatch::libdispatch::backend s_libdispatch;
    register_cxx_tests( "libdispatch", &s_libdispatch );
#endif

#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    static xdispatch::naive::backend s_naive;
    register_cxx_tests( "naive", &s_naive );
#endif

#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    QCoreApplication app( argc, argv );
    static xdispatch::qt5::backend s_qt5;
    register_cxx_tests( "qt5", &s_qt5 );
#endif

    register_platform_tests();
    register_signal_tests();

    ret = MU_main( argc, argv );

    return ret;
}
