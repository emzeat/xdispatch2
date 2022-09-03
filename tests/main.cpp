/*
 * main.cpp
 *
 * Copyright (c) 2011 - 2022 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 */

#include "munit/MUnit.h"
#include "cxx_tests.h"
#include "platform_tests.h"
#include "signal_tests.h"

#include "xdispatch/impl/ibackend.h"
#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
XDISPATCH_DECLARE_BACKEND(libdispatch)
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
XDISPATCH_DECLARE_BACKEND(naive)
#endif
#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    #include <QtCore/QCoreApplication>
XDISPATCH_DECLARE_BACKEND(qt5)
    #include "qt_tests.h"
#endif

void
print_log(const char* msg)
{
    printf("%s", msg);
    fflush(stdout);
}

/*
 The test program for testing the xdispatch2
 framework. The difficult task in here is
 that all functions dispatched can be started
 at any time. Thus each test has to run as own
 process, end when all functions were executed,
 and be supervised by the test application
 */

int
main(int argc, char* argv[])
{
    int ret = 0;
    MU_initFramework(print_log);

#if (defined BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
    register_cxx_tests("libdispatch",
                       libdispatch_backend_get_static_instance());
#endif

#if (defined BUILD_XDISPATCH2_BACKEND_NAIVE)
    register_cxx_tests("naive", naive_backend_get_static_instance());
#endif

#if (defined BUILD_XDISPATCH2_BACKEND_QT5)
    QCoreApplication app(argc, argv);
    register_cxx_tests("qt5", qt5_backend_get_static_instance());
    register_qt_tests();
#endif

    register_platform_tests();
    register_signal_tests();

    ret = MU_main(argc, argv);

    return ret;
}
