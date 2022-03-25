/*
 * cxx_dispatch_socket_notifier.cpp
 *
 * Copyright (c) 2008 - 2009 Apple Inc.
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

#include <xdispatch/dispatch.h>
#include "xdispatch/config.h"

#include <vector>
#include <cstdlib>
#include <cstdio>

#include "cxx_tests.h"
#include "platform_socketpair.h"

void
cxx_dispatch_notifier_read(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_notifier_read);

    static constexpr auto kPacket = 16;

    int fds[2] = { -1 };
    auto ret = platform_socketpair(fds);
    MU_ASSERT_NOT_EQUAL(ret, -1);

    // first make sure that a new socket cannot be read
    auto notifier = cxx_create_notifier(fds[1], xdispatch::notifier_type::READ);
    notifier.handler([](xdispatch::socket_t, xdispatch::notifier_type) {
        MU_FAIL("Should not be ready yet");
    });
    notifier.resume();
    MU_SLEEP(2);
    notifier.suspend();

    // use a new notifier to handle data actually posted to the socket
    notifier = cxx_create_notifier(fds[1], xdispatch::notifier_type::READ);
    notifier.handler(
      [fds](xdispatch::socket_t socket, xdispatch::notifier_type type) {
          MU_MESSAGE("READ handler call");
          MU_ASSERT_EQUAL(type, xdispatch::notifier_type::READ);
          MU_ASSERT_EQUAL(fds[1], socket);

          std::vector<char> buffer(64);
          auto actual = read(fds[1], &buffer[0], buffer.size());
          MU_ASSERT_EQUAL(actual, kPacket);

          MU_PASS("Seems to work");
      });
    notifier.resume();

    std::vector<char> buffer(kPacket);
    MU_ASSERT_EQUAL(kPacket, write(fds[0], &buffer[0], buffer.size()));

    cxx_exec();

    MU_END_TEST
}

void
cxx_dispatch_notifier_write(void* data)
{
    CXX_BEGIN_BACKEND_TEST(cxx_dispatch_notifier_write);

    static constexpr auto kPacket = 2048;

    int fds[2] = { -1 };
    auto ret = platform_socketpair(fds);
    MU_ASSERT_NOT_EQUAL(ret, -1);

    // write until the socket is saturated
    std::vector<char> buffer(kPacket);
    size_t written = 0;
    while (true) {
        const auto actual = write(fds[1], &buffer[0], kPacket);
        if (actual != kPacket) {
            break;
        }
        written += actual;
    }
    MU_MESSAGE("%lu bytes until saturated", written);

    // with a full socket we should not be able to write
    auto notifier =
      cxx_create_notifier(fds[1], xdispatch::notifier_type::WRITE);
    notifier.handler([](xdispatch::socket_t, xdispatch::notifier_type) {
        MU_FAIL("Should not be able to write yet");
    });
    notifier.resume();
    MU_SLEEP(2);
    notifier.suspend();

    // change the handler and read some to unblock
    notifier = cxx_create_notifier(fds[1], xdispatch::notifier_type::WRITE);
    notifier.handler(
      [fds](xdispatch::socket_t socket, xdispatch::notifier_type type) {
          MU_MESSAGE("WRITE handler call");
          MU_ASSERT_EQUAL(type, xdispatch::notifier_type::WRITE);
          MU_ASSERT_EQUAL(fds[1], socket);

          std::vector<char> buffer(kPacket);
          auto actual = write(fds[1], &buffer[0], kPacket);
          MU_ASSERT_EQUAL(actual, kPacket);
          MU_PASS("Seems to work");
      });
    notifier.resume();

    while (kPacket == read(fds[0], &buffer[0], buffer.size())) {
    }
    cxx_exec();

    MU_END_TEST;
}
