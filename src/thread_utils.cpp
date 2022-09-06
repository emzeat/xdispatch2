/*
 * thread_utils.cpp
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

#include "xdispatch_internal.h"
#include "thread_utils.h"
#include "trace_utils.h"

#include <cstdio>
#include <cstring>

#if (defined XDISPATCH2_HAVE_PRCTL)
    #include <sys/prctl.h>
#endif

#if (defined XDISPATCH2_HAVE_SETPRIORITY)
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
    #include <sys/syscall.h>
#endif

#if (defined XDISPATCH2_HAVE_SYSCTL)
    #include <sys/sysctl.h>
#endif

#if (defined XDISPATCH2_HAVE_SYSCONF)
    #include <unistd.h>
#endif

#if (defined XDISPATCH2_HAVE_IMMINTRIN_H)
    #include <immintrin.h>
#endif

#if (defined XDISPATCH2_HAVE_GET_SYSTEM_INFO)
    /* Reduces build time by omitting extra system headers */
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

// see
// https://docs.microsoft.com/de-de/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
const DWORD MS_VC_EXCEPTION = 0x406D1388;
    #pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO // NOLINT(modernize-use-using)
{
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
    #pragma pack(pop)

void
SetThreadName(DWORD dwThreadID, const char* threadName)
{
    static constexpr DWORD kThreadNameType = 0x100;
    THREADNAME_INFO info;
    info.dwType = kThreadNameType;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    #pragma warning(push)
    #pragma warning(disable : 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION,
                       0,
                       sizeof(info) / sizeof(ULONG_PTR),
                       (ULONG_PTR*)&info); // NOLINT(readability/casting)
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    #pragma warning(pop)
}

    #define XDISPATCH2_HAVE_SET_THREAD_NAME
#endif

__XDISPATCH_BEGIN_NAMESPACE

void
thread_utils::set_current_thread_name(const std::string& name)
{
#if (defined XDISPATCH2_HAVE_PTHREAD_SETNAME_NP)

    pthread_setname_np(name.c_str());

#endif

#if (defined XDISPATCH2_HAVE_PRCTL)

    prctl(PR_SET_NAME,
          (unsigned long)(name.c_str()),
          0,
          0,
          0); // NOLINT(runtime/int)

#endif

#if (defined XDISPATCH2_HAVE_SET_THREAD_NAME)

    if (trace_utils::is_debug_enabled() || IsDebuggerPresent()) {
        SetThreadName(GetCurrentThreadId(), name.c_str());
    };

#endif
}

void
thread_utils::set_current_thread_priority(queue_priority priority)
{
#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)

    const qos_class_t qos_class = map_priority_to_qos(priority);
    pthread_set_qos_class_self_np(qos_class, 0);

    #ifdef DEBUG
    qos_class_t qos_actual = QOS_CLASS_DEFAULT;
    pthread_get_qos_class_np(pthread_self(), &qos_actual, nullptr);
    XDISPATCH_ASSERT(qos_actual == qos_class);
    #endif

#elif (defined XDISPATCH2_HAVE_SETPRIORITY)

    int nice = 0;
    switch (priority) {
        case queue_priority::USER_INTERACTIVE:
            nice = 5; // NOLINT
            break;
        case queue_priority::USER_INITIATED:
            nice = 4; // NOLINT
            break;
        case queue_priority::UTILITY:
            nice = 3; // NOLINT
            break;
        case queue_priority::DEFAULT:
        case queue_priority::BACKGROUND:
            nice = 0; // NOLINT
            break;
    }
    const int tid = static_cast<int>(syscall(SYS_gettid));
    setpriority(PRIO_PROCESS, tid, nice);

#else

    static_cast<void>(priority);

#endif
}

#if (defined XDISPATCH2_HAVE_PTHREAD_SET_QOS_CLASS_SELF_NP)

qos_class_t
thread_utils::map_priority_to_qos(queue_priority priority)
{
    qos_class_t qos_class = QOS_CLASS_DEFAULT;
    switch (priority) {
        case queue_priority::USER_INTERACTIVE:
            qos_class = QOS_CLASS_USER_INTERACTIVE;
            break;
        case queue_priority::USER_INITIATED:
            qos_class = QOS_CLASS_USER_INITIATED;
            break;
        case queue_priority::UTILITY:
            qos_class = QOS_CLASS_UTILITY;
            break;
        case queue_priority::DEFAULT:
            qos_class = QOS_CLASS_DEFAULT;
            break;
        case queue_priority::BACKGROUND:
            qos_class = QOS_CLASS_BACKGROUND;
            break;
    }
    return qos_class;
}

#endif

size_t
thread_utils::system_thread_count()
{
    static auto sOverrideCount = [] {
        const char* override = std::getenv("XDISPATCH2_THREAD_COUNT");
        if (override) {
            try {
                auto threads = std::atoi(override);
                XDISPATCH_WARNING() << "Thread count forced to " << threads;
                return threads;
            } catch (std::exception& e) {
                XDISPATCH_WARNING()
                  << "Thread count could not be parsed: " << e.what();
                // pass, use actual readout below
            }
        }
        return 0;
    }();
    if (sOverrideCount > 0) {
        return sOverrideCount;
    }

#if (defined XDISPATCH2_HAVE_SYSCONF) &&                                       \
  (defined XDISPATCH2_HAVE_SYSCONF_SC_NPROCESSORS_ONLN)
    const auto nprocessors = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocessors < 0) {
        XDISPATCH_TRACE() << "system_thread_count failed: " << strerror(errno);
    } else {
        return nprocessors;
    }
#endif

#if (defined XDISPATCH2_HAVE_SYSCTL) && (defined XDISPATCH2_HAVE_SYSCTL_HW_NCPU)
    int value = 0;
    size_t length = sizeof(value);
    int mib[] = { CTL_HW, HW_NCPU };

    if (-1 == sysctl(mib, 2, &value, &length, nullptr, 0)) {
        XDISPATCH_TRACE() << "system_thread_count failed: " << strerror(errno);
    } else {
        return value;
    }
#endif

#if (defined XDISPATCH2_HAVE_GET_SYSTEM_INFO)
    SYSTEM_INFO systeminfo;
    GetSystemInfo(&systeminfo);
    if (systeminfo.dwNumberOfProcessors > 0) {
        return static_cast<size_t>(systeminfo.dwNumberOfProcessors);
    }
#endif

    // default
    XDISPATCH_TRACE()
      << "system_thread_count using hardcoded default on this platform";
    return 2;
}

void
thread_utils::cpu_relax()
{
#if defined(XDISPATCH2_HAVE_IMMINTRIN_H)
    _mm_pause();
#elif (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) ||      \
  defined(__arm64__)
    __asm__("yield");
#else
    __asm__("");
#endif
}

__XDISPATCH_END_NAMESPACE
