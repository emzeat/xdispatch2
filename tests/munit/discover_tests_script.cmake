#
# discover_tests_script.cmake
#
# Copyright (c) 2011 - 2023 Marius Zwicker
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

# macro to discover tests declared via munit and register them
# to be executed via ctest in a similar way as gtest_discover_tests


#set(TEST_TARGET "")
#set(TEST_EXECUTABLE "/Volumes/Data/Development/footo.git/sharedlibs/xdispatch2/build/clang-ninja-release/bin/xdispatch2_tests")
#set(TEST_WORKING_DIR "/Volumes/Data/Development/footo.git/sharedlibs/xdispatch2/build/clang-ninja-release/bin")
#set(TEST_PREFIX "")
#set(TEST_SUFFIX "")
#set(CTEST_FILE "/Volumes/Data/Development/footo.git/sharedlibs/xdispatch2/build/clang-ninja-release/tests/xdispatch2_tests_munit_tests.cmake")

execute_process(
    COMMAND ${TEST_EXECUTABLE} -l
    OUTPUT_VARIABLE TEST_LIST_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

string(REGEX MATCHALL "[0-9]+\t[^\n\r]+" TEST_LIST ${TEST_LIST_OUTPUT})
foreach(TEST IN LISTS TEST_LIST)
    string(REGEX MATCH "([0-9]+)\t(.+)" TEST_DETAIL ${TEST})
    list(APPEND CTEST_REGISTRATION "add_test(\"${TEST_PREFIX}${CMAKE_MATCH_2}${TEST_SUFFIX}\"\n${TEST_EXECUTABLE} -t ${CMAKE_MATCH_1}\n)")
endforeach()

list(JOIN CTEST_REGISTRATION "\n" CTEST_REGISTRATION)
file(WRITE "${CTEST_FILE}"
    "${CTEST_REGISTRATION}"
)
