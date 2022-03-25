/*
 * platform_socketpair.h
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

#ifndef PLATFORM_SOCKETPAIR_H_
#define PLATFORM_SOCKETPAIR_H_

#include <xdispatch/dispatch.h>
#include "xdispatch/config.h"

#include <vector>
#include <cstdlib>
#include <cstdio>

#include "munit/MUnit_tools.h"

#if (defined XDISPATCH2_HAVE_SOCKETPAIR)
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <sys/un.h>
#elif (defined XDISPATCH2_HAVE_WINSOCK2)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <io.h>
#else
    #error "Missing support for socketpair()"
#endif

inline int
platform_socketpair(xdispatch::socket_t sockets[2])
{
#if (defined XDISPATCH2_HAVE_SOCKETPAIR)
    auto ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets);
    if (ret != -1) {
        for (int i = 0; i < 2; ++i) {
            const auto flags = fcntl(sockets[i], F_GETFL, 0);
            fcntl(sockets[i], F_SETFL, flags | O_NONBLOCK);
        }
    }
    return ret;
#elif (defined XDISPATCH2_HAVE_WINSOCK2)
    static auto sWSA = [] {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }();

    auto listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        MU_MESSAGE("socket(..) failed: %i", WSAGetLastError());
        return SOCKET_ERROR;
    }

    union
    {
        struct sockaddr_in inaddr;
        struct sockaddr addr;
    } a;
    memset(&a, 0, sizeof(a));
    a.inaddr.sin_family = AF_INET;
    a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.inaddr.sin_port = 0;

    sockets[0] = INVALID_SOCKET;
    sockets[1] = INVALID_SOCKET;
    do {
        const int reuse = 1;
        if (setsockopt(listener,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       (char*)&reuse,
                       (socklen_t)sizeof(reuse)) == -1) {
            break;
        }
        if (bind(listener, &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR) {
            break;
        }
        socklen_t namelen = sizeof(a.inaddr);
        if (getsockname(listener, &a.addr, &namelen) == SOCKET_ERROR) {
            break;
        }
        if (listen(listener, 1) == SOCKET_ERROR) {
            break;
        }

        const DWORD flags = WSA_FLAG_OVERLAPPED;
        sockets[0] = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, flags);
        if (sockets[0] == INVALID_SOCKET) {
            break;
        }
        if (connect(sockets[0], &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR) {
            break;
        }

        sockets[1] = accept(listener, nullptr, nullptr);
        if (sockets[1] == INVALID_SOCKET) {
            break;
        }

        closesocket(listener);
        for (int i = 0; i < 2; ++i) {
            u_long param = 1;
            ioctlsocket(sockets[i], FIONBIO, &param);
        }
        return 0;

    } while (0);

    auto e = WSAGetLastError();
    closesocket(listener);
    closesocket(sockets[0]);
    closesocket(sockets[1]);
    WSASetLastError(e);

    MU_MESSAGE("platform_socketpair(..) failed: %i", WSAGetLastError());
    return SOCKET_ERROR;
#endif
}

#if (defined XDISPATCH2_HAVE_WINSOCK2)

inline int
write(xdispatch::socket_t s, const char* data, size_t len)
{
    const int ret = send(s, data, static_cast<int>(len), 0);
    if (ret < 0) {
        errno = WSAGetLastError();
        switch (errno) {
            case WSAEINTR:
            case WSAEWOULDBLOCK:
                errno = EAGAIN;
                break;
            default:
                break;
        }
    }
    return ret;
}

inline int
read(xdispatch::socket_t s, char* data, size_t len)
{
    const int ret = recv(s, data, static_cast<int>(len), 0);
    if (ret < 0) {
        errno = WSAGetLastError();
        switch (errno) {
            case WSAEINTR:
            case WSAEWOULDBLOCK:
                errno = EAGAIN;
                break;
            default:
                break;
        }
    }
    return ret;
}

#endif

#endif /* PLATFORM_SOCKETPAIR_H_ */
