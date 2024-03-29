/*
 * MUnit_tools.h
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
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

#ifndef MUNIT_TOOLS_H_
#define MUNIT_TOOLS_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN 1
    #include <windows.h>
    #define MU_SLEEP(T) Sleep(1000 * (T))
#else // ifdef _WIN32
    #include <unistd.h>
    #define MU_SLEEP(T) sleep(T)
#endif // ifdef _WIN32

#include "list.h"
#include "MUnit_runner.h"

typedef void (*mu_test_func)(void*);

extern item_t* suite;
extern MU_messageHandler msg_handler;

#define MU_CONCAT_(X, Y) X##Y

#define MU_CONCAT(X, Y) MU_CONCAT_(X, Y)

#define MU_REGISTER_TEST(NAME) _register_suite(NAME, #NAME, 0)

#ifdef __cplusplus
    #define MU_REGISTER_TEST_INSTANCE(INSTANCE, NAME, DATA)                    \
        const auto MU_CONCAT(sInstance, __LINE__) =                            \
          std::string(INSTANCE) + "__" + #NAME;                                \
        _register_suite(NAME, MU_CONCAT(sInstance, __LINE__).c_str(), DATA)
#endif

void
_register_suite(mu_test_func function, const char* name, void* user);

#define MU_BEGIN_TEST(NAME) _begin_test(#NAME)
void
_begin_test(const char* name);

#define MU_END_TEST MU_FAIL("!! INCOMPLETE TEST !!");

#define MU_MESSAGE(format, ...)                                                \
    _print_message("\t-> " format "\n", ##__VA_ARGS__)
static void
_print_message(const char* format, ...)
{
    static char was_called = 0;
    va_list params;
    char tmp[512];

    if (!was_called) {
        was_called = 1;
        msg_handler("\n");
    }

    va_start(params, format);
#ifdef WIN32_VS
    vsprintf_s(tmp, 510, format, params);
#else
    vsnprintf(tmp, sizeof(tmp) / sizeof(tmp[0]), format, params);
#endif
    va_end(params);
    msg_handler(tmp);
} // _print_message

#endif /* MUNIT_TOOLS_H_ */
