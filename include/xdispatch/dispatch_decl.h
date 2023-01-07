/*
 * dispatch_decl.h
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

#ifndef XDISPATCH_DECL_H_
#define XDISPATCH_DECL_H_

#include "xdispatch/platform.h"
#include "xdispatch/config.h"

#if XDISPATCH_COMPILER_MSVC
    #pragma warning(                                                           \
      disable : 4251) /* disable warning C4251 - * requires dll-interface */
    #if (defined XDISPATCH2_BUILD_SHARED)
        #if (defined XDISPATCH_MAKEDLL)
            #define XDISPATCH_EXPORT __declspec(dllexport)
        #else
            #define XDISPATCH_EXPORT __declspec(dllimport)
        #endif
    #else
        #define XDISPATCH_EXPORT
    #endif
    #define XDISPATCH_DEPRECATED(F) __declspec(deprecated) F
#elif XDISPATCH_COMPILER_GCC || XDISPATCH_COMPILER_CLANG
    #if (defined XDISPATCH2_BUILD_SHARED)
        #define XDISPATCH_EXPORT __attribute__((__visibility__("default")))
    #else
        #define XDISPATCH_EXPORT
    #endif
    #define XDISPATCH_DEPRECATED(F) __attribute__((__deprecated__)) F
#endif // if XDISPATCH_COMPILER_MSVC2010 || XDISPATCH_COMPILER_MSVC2008SP1

#define __XDISPATCH_BEGIN_NAMESPACE namespace xdispatch {
#define __XDISPATCH_END_NAMESPACE }

/*
#ifdef _WIN32
# pragma warning(default: 4251) // re-enable warning C4251 - we do not want to
influence other code #endif
*/

/**
 * @addtogroup xdispatch
 * @{
 */

__XDISPATCH_BEGIN_NAMESPACE

/**
  The number of nanoseconds per second
  */
static constexpr std::chrono::nanoseconds nsec_per_sec =
  std::chrono::seconds(1);
/**
  The number of nanoseconds per millisecond
  */
static constexpr std::chrono::nanoseconds nsec_per_msec =
  std::chrono::milliseconds(1);
/**
  The number of nanoseconds per microsecond
  */
static const std::chrono::nanoseconds nsec_per_usec =
  std::chrono::microseconds(1);
/**
  The number of microseconds per second
  */
static const std::chrono::microseconds usec_per_sec = std::chrono::seconds(1);

/**
    Priority classes used to classify operations executed
    on a queue to help the system with managing resources
    */
enum class queue_priority
{
    DEFAULT, //!< Default classification for the given platform, used when no
             //!< other priority is set

    USER_INTERACTIVE, //!< Operations affecting the user interface rendering
    USER_INITIATED,   //!< Operations not impacting the user interface but
                      //!< blocking the user from continueing
    UTILITY,   //!< Operations for ongoing operations that the user started or
               //!< gets informed about
    BACKGROUND //!< Operations that perform utility tasks in the background
               //!< which are free to take longer
};

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_DECL_H_ */
