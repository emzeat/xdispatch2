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
include(CheckIncludeFile)
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
set(CMAKE_REQUIRED_LIBRARIES dl)
check_symbol_exists( dlsym "dlfcn.h" XDISPATCH2_HAVE_DLSYM )
unset(CMAKE_REQUIRED_LIBRARIES)
check_symbol_exists( GetProcAddress "windows.h" XDISPATCH2_HAVE_GET_PROC_ADDRESS )
check_include_file( "immintrin.h" XDISPATCH2_HAVE_IMMINTRIN_H )

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
mz_add_definition(XDISPATCH_MAKEDLL)
if( MZ_IOS )
    set( XDISPATCH_BUILD_TYPE STATIC )
else()
    set( XDISPATCH_BUILD_TYPE SHARED )
endif()

# generate the build time configuration header
configure_file(config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/xdispatch/config.h
)

# core lib
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB CXX
    include/xdispatch/*.h
    include/xdispatch/impl/*.h
    include/xdispatch/dispatch
    src/*.cpp
    src/*.h
)
file(GLOB CXX_INCL
    include/xdispatch/*.h
    include/xdispatch/dispatch
)
if( XDISPATCH2_HAVE_DLSYM )
    set(LIBS ${LIBS}
        dl
    )
endif()

# naive backend (required)
if( BUILD_XDISPATCH2_BACKEND_NAIVE )
    find_package(Threads REQUIRED)
    file(GLOB CXX_NAIVE
        src/naive/*.cpp
        src/naive/*.h
    )
    if( XDISPATCH2_HAVE_WINSOCK2 )
        set(LIBS ${LIBS}
            Ws2_32
        )
    endif()
    set(LIBS ${LIBS}
        Threads::Threads
    )
endif()

# main library target (always including naive)
add_library( xdispatch ${XDISPATCH_BUILD_TYPE}
    ${CXX}
    ${CXX_NAIVE}
)
mz_target_props( xdispatch )
mz_auto_format( xdispatch )
target_link_libraries( xdispatch
    PRIVATE ${LIBS}
)
set_target_properties( xdispatch PROPERTIES
    VERSION ${XDISPATCH2_VERSION}
    SOVERSION ${XDISPATCH2_VERSION}
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}/include"
)
target_include_directories( xdispatch
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# helper macro to install an xdispatch target
macro(install_xdispatch_target TGT_NAME)
    if(MZ_IOS)
        install(TARGETS ${TGT_NAME}
                    LIBRARY DESTINATION lib
                    RUNTIME DESTINATION bin
                    ARCHIVE DESTINATION lib)
        install(DIRECTORY include/xdispatch
                    DESTINATION include
                    PATTERN "*.git" EXCLUDE)

    elseif(MZ_MACOS AND BUILD_XDISPATCH2_AS_FRAMEWORK)
        set_target_properties( ${TGT_NAME} PROPERTIES
            FRAMEWORK TRUE
            MACOSX_FRAMEWORK_IDENTIFIER "de.emzeat.${TGT_NAME}"
            MACOSX_FRAMEWORK_BUNDLE_VERSION "${XDISPATCH2_VERSION}"
            PUBLIC_HEADER "${CXX_INCL}"
        )

        install(TARGETS ${TGT_NAME}
                    COMPONENT lib${TGT_NAME}
                    FRAMEWORK DESTINATION "Library/Frameworks")
    else()
        install(TARGETS ${TGT_NAME}
                    COMPONENT lib${TGT_NAME}
                    LIBRARY DESTINATION lib
                    RUNTIME DESTINATION bin
                    ARCHIVE DESTINATION lib)
        install(DIRECTORY include/xdispatch
                    DESTINATION include
                    COMPONENT lib${TGT_NAME}-dev)
    endif()
endmacro()
install_xdispatch_target(xdispatch)

# helper macro to define a custom backend
macro(add_xdispatch_backend SUFFIX)
    set(_mz3_multiValueArgs
    )
    cmake_parse_arguments( _xb
        "BUILTIN" # options
        "" # one value
        "SOURCES;LIBS" # multi value
        ${ARGN}
    )

    mz_message("Building backend: ${SUFFIX}")
    if(_xb_BUILTIN)
        add_library( xdispatch_${SUFFIX} ALIAS
            xdispatch
        )
        target_sources( xdispatch
            PRIVATE ${_xb_SOURCES}
        )
        target_link_libraries( xdispatch
            PRIVATE ${_xb_LIBS}
        )
    else()
        add_library( xdispatch_${SUFFIX} ${XDISPATCH_BUILD_TYPE}
            ${_xb_SOURCES}
        )
        mz_target_props( xdispatch_${SUFFIX} )
        mz_auto_format( xdispatch_${SUFFIX} )
        target_link_libraries( xdispatch_${SUFFIX}
            PUBLIC xdispatch
            PRIVATE ${_xb_LIBS}
        )
        set_target_properties( xdispatch_${SUFFIX} PROPERTIES
            VERSION ${XDISPATCH2_VERSION}
            SOVERSION ${XDISPATCH2_VERSION}
        )
        install_xdispatch_target(xdispatch_${SUFFIX})
    endif()
endmacro()

# libdispatch backend
if( BUILD_XDISPATCH2_BACKEND_LIBDISPATCH )
    file(GLOB CXX_LIBDISPATCH
        src/libdispatch/*.cpp
        src/libdispatch/*.h
    )
    # use libdispatch automatically, it causes no additional dependencies or license limits
    add_xdispatch_backend(libdispatch
        BUILTIN
        SOURCES ${CXX_LIBDISPATCH}
    )
endif()

# qt backend
if( BUILD_XDISPATCH2_BACKEND_QT5 )
    file(GLOB CXX_QT5
        src/qt5/*.cpp
        src/qt5/*.h
    )
    mz_qt_auto_moc(MOC_QT5 ${CXX_QT5})
    if(BUILD_XDISPATCH2_BACKEND_LIBDISPATCH)
        set(LIBS_QT5 xdispatch_libdispatch)
    endif()
    # not a builtin backend, linking with qt incurrs LGPL or other license
    # limitations and as such should be made explicit
    add_xdispatch_backend(qt5
        SOURCES ${CXX_QT5} ${MOC_QT5}
        LIBS Qt5::Core ${LIBS_QT5}
    )
endif()


if(BUILD_XDISPATCH2_TESTS)
    mz_add_executable(xdispatch2_tests tests)
endif()
