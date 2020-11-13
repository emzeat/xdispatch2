#
# CMakeLists.txt
#
# Copyright (c) 2011-2018 MLBA-Team
# All rights reserved.
#
# @LICENSE_HEADER_START@
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
# @LICENSE_HEADER_END@
#

cmake_minimum_required(VERSION 2.8)
include(CheckSymbolExists)
project(xdispatch2)

# build scripts
if(NOT HAS_MZ_GLOBAL)
    include(build/global.cmake)
    include(build/macros.cmake)
    include(build/3rdparty.cmake)
endif()

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

# build options
set(BUILD_XDISPATCH2_BACKEND_NAIVE ON)
option(BUILD_XDISPATCH2_BACKEND_QT5 "Build the qt5 backend" ON)
option(BUILD_XDISPATCH2_TESTS "Build the xdispatch2 test target" OFF)
if( XDISPATCH2_HAVE_LIBDISPATCH_NATIVE )
    option(BUILD_XDISPATCH2_BACKEND_LIBDISPATCH "Build the libdispatch backend" ON)
else()
    option(BUILD_XDISPATCH2_BACKEND_LIBDISPATCH "Build the libdispatch backend" OFF)
endif()


# configuration and version information
find_package(Git)
if(GIT_FOUND)
    # we use a short hash
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log --pretty=format:%h -n 1
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE XDISPATCH_WC_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE XDISPATCH_WC_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT XDISPATCHAM_WC_TAG)
        set(XDISPATCH_WC_TAG "alpha-${XDISPATCH_WC_REVISION}")
    endif()
endif()
set( XDISPATCH_WC_TAG "2.${XDISPATCH_WC_TAG}" )
mz_message( "xdispatch version ${XDISPATCH_WC_TAG}" )


# dependencies
if( BUILD_XDISPATCH2_BACKEND_QT5 AND NOT TARGET qt5-core )
    mz_add_library(qt5 3rdparty/qt5)
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
endif()

# libdispatch backend
if( BUILD_XDISPATCH2_BACKEND_LIBDISPATCH )
    file(GLOB CXX_LIBDISPATCH
        src/libdispatch/*.cpp
        src/libdispatch/*.h
    )
endif()

# qt backend
if( BUILD_XDISPATCH2_BACKEND_QT5 )
    file(GLOB CXX_QT5
        src/qt5/*.cpp
        src/qt5/*.h
    )
    mz_qt_auto_moc(MOC_QT5 ${CXX_QT5})
    list(APPEND CXX_QT5 ${MOC_QT5})
    set(LIBS ${LIBS}
        qt5-core
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
    VERSION ${XDISPATCH_WC_TAG}
    SOVERSION ${XDISPATCH_WC_TAG}
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

elseif(MZ_MACOS)
    set_target_properties( xdispatch PROPERTIES
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER "de.mlba-team.xdispatch"
        MACOSX_FRAMEWORK_BUNDLE_VERSION "${XDISPATCH_WC_TAG}"
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
