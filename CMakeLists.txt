#
# CMakeLists.txt
#
# Copyright (c) 2011 - 2022 Marius Zwicker
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.10)
include(CheckSymbolExists)
project(xdispatch2)

# build scripts
if(NOT HAS_MZ_GLOBAL)
    include(build/global.cmake)
    include(build/macros.cmake)
    include(build/conan.cmake)
endif()

# enable linting
include(build/linting.cmake)

# check for platform features
check_symbol_exists( dispatch_queue_create "dispatch/dispatch.h" XDISPATCH2_HAVE_LIBDISPATCH_NATIVE )
check_symbol_exists( pthread_setname_np "pthread.h" XDISPATCH2_HAVE_PTHREAD_SETNAME_NP )
check_symbol_exists( pthread_set_qos_class_self_np "pthread.h;sys/qos.h" XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP )
check_symbol_exists( prctl "sys/prctl.h" XDISPATCH2_HAVE_PRCTL )
check_symbol_exists( setpriority "sys/resource.h;sys/syscall.h" XDISPATCH2_HAVE_SETPRIORITY )
check_symbol_exists( sysconf "unistd.h" XDISPATCH2_HAVE_SYSCONF )
check_symbol_exists( _SC_NPROCESSORS_ONLN "unistd.h" XDISPATCH2_HAVE_SYSCONF_SC_NPROCESSORS_ONLN )
check_symbol_exists( sysctl "sys/sysctl.h" XDISPATCH2_HAVE_SYSCTL )
check_symbol_exists( HW_NCPU "sys/sysctl.h" XDISPATCH2_HAVE_SYSCTL_HW_NCPU )
check_symbol_exists( GetSystemInfo "windows.h" XDISPATCH2_HAVE_GET_SYSTEM_INFO )
check_symbol_exists( socketpair "sys/socket.h" XDISPATCH2_HAVE_SOCKETPAIR )
check_symbol_exists( WSASocket "winsock2.h" XDISPATCH2_HAVE_WINSOCK2 )

# build options
set(BUILD_XDISPATCH2_BACKEND_NAIVE ON)
option(BUILD_XDISPATCH2_BACKEND_QT5 "Build the qt5 backend" ON)
option(BUILD_XDISPATCH2_TESTS "Build the xdispatch2 test target" OFF)
if( XDISPATCH2_HAVE_LIBDISPATCH_NATIVE )
    option(BUILD_XDISPATCH2_BACKEND_LIBDISPATCH "Build the libdispatch backend" ON)
else()
    option(BUILD_XDISPATCH2_BACKEND_LIBDISPATCH "Build the libdispatch backend" OFF)
endif()
if( MZ_MACOS )
    option(BUILD_XDISPATCH2_AS_FRAMEWORK "Build xdispatch2 as framework" ON)
endif()


# configuration and version information
include(build/semver.cmake)
mz_determine_sem_ver(PREFIX "XDISPATCH2")
mz_message("xdispatch version ${XDISPATCH2_VERSION}")


# dependencies
if( BUILD_XDISPATCH2_BACKEND_QT5 )
    include(build/qt5.cmake)
endif()

# we hide all unneeded symbols
if( NOT MZ_WINDOWS )
    mz_add_flag(GCC -fvisibility=hidden)
endif()

# generate the build time configuration header
configure_file(config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/xdispatch/config.h
)

# core lib
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB CXX
    include/xdispatch/*.h
    include/xdispatch/dispatch
    src/*.cpp
    src/*.h
)
file(GLOB CXX_INCL
    include/xdispatch/*.h
    include/xdispatch/dispatch
)

# naive backend (required)
if( BUILD_XDISPATCH2_BACKEND_NAIVE )
    file(GLOB CXX_NAIVE
        src/naive/*.cpp
        src/naive/*.h
    )
    if( XDISPATCH2_HAVE_WINSOCK2 )
        set(LIBS ${LIBS}
            Ws2_32
        )
    endif()
endif()

# libdispatch backend
if( BUILD_XDISPATCH2_BACKEND_LIBDISPATCH )
    mz_message("Building backend: libdispatch")
    file(GLOB CXX_LIBDISPATCH
        src/libdispatch/*.cpp
        src/libdispatch/*.h
    )
endif()

# qt backend
if( BUILD_XDISPATCH2_BACKEND_QT5 )
    mz_message("Building backend: qt5")
    file(GLOB CXX_QT5
        src/qt5/*.cpp
        src/qt5/*.h
    )
    mz_qt_auto_moc(MOC_QT5 ${CXX_QT5})
    list(APPEND CXX_QT5 ${MOC_QT5})
    set(LIBS ${LIBS}
        Qt5::Core
    )
endif()

# add new target xdispatch
mz_add_definition(XDISPATCH_MAKEDLL)
if( MZ_IOS )
    set( XDISPATCH_BUILD_TYPE STATIC )
else()
    set( XDISPATCH_BUILD_TYPE SHARED )
endif()

add_library( xdispatch ${XDISPATCH_BUILD_TYPE}
    ${CXX}
    ${CXX_NAIVE}
    ${CXX_LIBDISPATCH}
    ${CXX_QT5}
    ${RES_FILE}
)
mz_target_props( xdispatch )
mz_auto_format( xdispatch )

# link this target with all needed libraries
message("-- linking xdispatch with: ${LIBS}")
target_link_libraries( xdispatch ${LIBS})

set_target_properties( xdispatch PROPERTIES
    VERSION ${XDISPATCH2_VERSION}
    SOVERSION ${XDISPATCH2_VERSION}
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}/include"
)

target_include_directories( xdispatch
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MZ_IOS)
    install(TARGETS xdispatch
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin
                ARCHIVE DESTINATION lib)
    install(DIRECTORY include/xdispatch
                DESTINATION include
                PATTERN "*.git" EXCLUDE)

elseif(MZ_MACOS AND BUILD_XDISPATCH2_AS_FRAMEWORK)
    set_target_properties( xdispatch PROPERTIES
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER "de.emzeat.xdispatch"
        MACOSX_FRAMEWORK_BUNDLE_VERSION "${XDISPATCH2_VERSION}"
        PUBLIC_HEADER "${CXX_INCL}"
    )

    install(TARGETS xdispatch
                COMPONENT libxdispatch
                FRAMEWORK DESTINATION "Library/Frameworks")
else()
    install(TARGETS xdispatch
                COMPONENT libxdispatch
                LIBRARY DESTINATION lib
                RUNTIME DESTINATION bin
                ARCHIVE DESTINATION lib)
    install(DIRECTORY include/xdispatch
                DESTINATION include
                COMPONENT libxdispatch-dev)
endif()

if(BUILD_XDISPATCH2_TESTS)
    mz_add_executable(xdispatch2_tests tests)
endif()
