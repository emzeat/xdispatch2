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

# test runner spawning sub processes
mz_add_library(munit munit)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR})
file( GLOB TEST_CXX
  cxx_*.cpp
  cxx_*.h
  platform_*.cpp
  platform_*.h
  signal_*.cpp
  signal_*.h
)

if( BUILD_XDISPATCH2_BACKEND_QT5 )
    file( GLOB TEST_QT
      qt_*.cpp
      qt_*.h
    )
endif()

# includes
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

if( MINGW )
    set( RES_FILES tests.rc )
endif()

# add new exec target
if( MZ_IOS )
    add_library( xdispatch2_tests STATIC
        main_ios.cpp
        ${TEST_CXX}
        ${TEST_QT}
        ${RES_FILES}
    )
else()
    add_executable( xdispatch2_tests
        main.cpp
        ${TEST_CXX}
        ${TEST_QT}
        ${TEST_LIBDISPATCH}
        ${RES_FILES}
    )
endif()

# manifest (somehow we need one when using mingw - but only for this binary !?
if( MINGW )
    # create the manifest for the first time
    execute_process(
      COMMAND windres --input ${CMAKE_CURRENT_SOURCE_DIR}/tests.rc --output tests.res --output-format=coff
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  # and all other times automatically
  add_custom_command( TARGET xdispatch2_tests PRE_LINK
    COMMAND windres --input ${CMAKE_CURRENT_SOURCE_DIR}/tests.rc --output tests.res --output-format=coff
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
  set(RES_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/tests.res)
endif()

# link this target with all needed libraries
target_link_libraries( xdispatch2_tests
    xdispatch
    munit
    ${RES_OBJECT}
)
if( BUILD_XDISPATCH2_BACKEND_QT5 )
    target_link_libraries( xdispatch2_tests
        qt5-gui
        qt5-widgets
    )
endif()
mz_target_props( xdispatch2_tests )
mz_auto_format( xdispatch2_tests )

# copy some benchmarking scripts
file( COPY
    xdispatch2_benchmarks.sh
    DESTINATION ${EXECUTABLE_OUTPUT_PATH}
    FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_READ GROUP_EXECUTE
)
