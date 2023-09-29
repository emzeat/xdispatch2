#
# discover_tests.cmake
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


function(munit_discover_tests TARGET)
  cmake_parse_arguments(
    ""
    ""
    "TEST_PREFIX;TEST_SUFFIX;WORKING_DIRECTORY"
    ""
    ${ARGN}
  )

  if(NOT _WORKING_DIRECTORY)
    set(_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endif()


  # Define rule to generate test list for aforementioned test executable
  set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_munit")
  set(ctest_include_file "${ctest_file_base}_include.cmake")
  set(ctest_tests_file "${ctest_file_base}_tests.cmake")

  add_custom_command(
      TARGET ${TARGET} POST_BUILD
      BYPRODUCTS "${ctest_tests_file}"
      COMMAND "${CMAKE_COMMAND}"
              -D "TEST_TARGET=${TARGET}"
              -D "TEST_EXECUTABLE=$<TARGET_FILE:${TARGET}>"
              -D "TEST_WORKING_DIR=${_WORKING_DIRECTORY}"
              -D "TEST_PREFIX=${_TEST_PREFIX}"
              -D "TEST_SUFFIX=${_TEST_SUFFIX}"
              -D "CTEST_FILE=${ctest_tests_file}"
              -P "${CMAKE_CURRENT_LIST_DIR}/munit/discover_tests_script.cmake"
      VERBATIM
  )

  file(WRITE "${ctest_include_file}"
    "if(EXISTS \"${ctest_tests_file}\")\n"
    "  include(\"${ctest_tests_file}\")\n"
    "else()\n"
    "  add_test(${TARGET}_NOT_BUILT ${TARGET}_NOT_BUILT)\n"
    "endif()\n"
  )

  # Add discovered tests to directory TEST_INCLUDE_FILES
  set_property(DIRECTORY
    APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}"
  )

endfunction()
