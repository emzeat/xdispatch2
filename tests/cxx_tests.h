/*
 * cxx_tests.h
 *
 * Copyright (c) 2008-2009 Apple Inc.
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

#ifndef CXX_TESTS_H_
#define CXX_TESTS_H_

#include <xdispatch/ibackend.h>

#include "munit/MUnit.h"

void
register_cxx_tests(const char* name, xdispatch::ibackend* backend);

void
cxx_begin_test(void* data);

xdispatch::queue
cxx_main_queue();

xdispatch::queue
cxx_global_queue(
  xdispatch::queue_priority priority = xdispatch::queue_priority::DEFAULT);

xdispatch::queue
cxx_create_queue(
  const char* label,
  xdispatch::queue_priority priority = xdispatch::queue_priority::DEFAULT);

xdispatch::group
cxx_create_group();

xdispatch::timer
cxx_create_timer(const xdispatch::queue& queue = cxx_global_queue());

void
cxx_exec();

#define CXX_BEGIN_BACKEND_TEST(NAME)                                           \
    MU_BEGIN_TEST(NAME);                                                       \
    cxx_begin_test(data)

#endif /* CXX_TESTS_H_ */
