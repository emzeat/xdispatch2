/*
* platform.h
*
* Copyright (c) 2011-2018 MLBA-Team
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

#ifndef XDISPATCH_PLATFORM_H_
#define XDISPATCH_PLATFORM_H_


// detect compilers

#if defined __clang__
    #define XDISPATCH_COMPILER_CLANG 1
    #define XDISPATCH_COMPILER "clang"
#endif

#if defined __GNUC__ && ( !defined __clang__ )
    #define XDISPATCH_COMPILER_GCC 1
    #define XDISPATCH_COMPILER "gcc"
#endif

#if defined _MSC_VER
    #define XDISPATCH_COMPILER_MSVC 1
    #define XDISPATCH_COMPILER "Visual Studio"
#endif

#ifndef XDISPATCH_COMPILER
    # error "Unsupported compiler version"
#endif

#include <memory>
#include <chrono>
#include <string>
#include <atomic>
#include <type_traits>

#ifdef _WIN32
    #define XDISPATCH_WARN_UNUSED_RETURN(Ret) Ret
#else
    #define XDISPATCH_WARN_UNUSED_RETURN(Ret) Ret __attribute__((warn_unused_result))
#endif

#endif // XDISPATCH_PLATFORM_H_
