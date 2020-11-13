/*
* main_ios.c
*
* Copyright (c) 2011 MLBA.
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
    #include "qt_tests.h"
#endif

void null_printer(const char* _unused_){

}

/*
 The test library for testing the dispatch
 framework. See also main.c
 */

// call this function from within your
// ios program to run all tests.
// Pass a message handler to print all messages
int run_dispatch_tests(int argc, char* argv[], MU_messageHandler handler) {
	int ret = 0;

    if( handler == NULL )
        handler = null_printer;

	MU_initFramework( handler );

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
    register_qt_tests();
#endif

    register_platform_tests();
    register_signal_tests();

	ret = MU_main(argc,argv);

	return ret;
}
