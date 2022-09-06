/*
 * symbol_utils.h
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

#ifndef XDISPATCH_SYMBOL_UTILS_H_
#define XDISPATCH_SYMBOL_UTILS_H_

#include "xdispatch/dispatch.h"
#include "trace_utils.h"

#if (defined XDISPATCH2_HAVE_DLSYM)
    #include <dlfcn.h>
#elif (defined XDISPATCH2_HAVE_GET_PROC_ADDRESS)
    /* Reduces build time by omitting extra system headers */
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <Psapi.h>
    #include <vector>
#endif

__XDISPATCH_BEGIN_NAMESPACE

class symbol_utils
{
public:
    /**
      @brief Try to resolve the given symbol using currently loaded modules
      @param symbol The name of the symbol to be resolved
      @param hint
      @tparam Signature The signature of the symbol to be resolved
      @return A function pointer to the resolved symbol or null if not found
     */
    template<typename Signature>
    static Signature* resolve(const char* symbol)
    {
        XDISPATCH_TRACE() << "symbol_utils::resolve(" << symbol << ")";

#if (defined XDISPATCH2_HAVE_DLSYM)
        auto* handle = dlopen(nullptr, RTLD_LAZY);
        if (handle) {
            auto* function =
              reinterpret_cast<Signature*>(dlsym(handle, symbol));
            dlclose(handle);
            return function;
        }
#elif (defined XDISPATCH2_HAVE_GET_PROC_ADDRESS)
        // first search all loaded modules
        DWORD modCount = 0;
        HANDLE process = GetCurrentProcess();
        if (EnumProcessModules(process, nullptr, 0, &modCount) &&
            modCount > 0) {
            std::vector<HMODULE> modules(modCount, nullptr);
            if (EnumProcessModules(
                  process, modules.data(), modules.size(), &modCount)) {
                for (auto handle : modules) {
                    auto* function = reinterpret_cast<Signature*>(
                      GetProcAddress(handle, symbol));
                    if (function) {
                        return function;
                    }
                }
            }
        }
        // if none search this process, we might be statically linked
        auto* handle = GetModuleHandleA(nullptr);
        if (handle) {
            return reinterpret_cast<Signature*>(GetProcAddress(handle, symbol));
        }
#else
    #error "Resolving functions was not implemented on this platform"
#endif
        return nullptr;
    }

    symbol_utils() = delete;
};

__XDISPATCH_END_NAMESPACE

#endif // XDISPATCH_SYMBOL_UTILS_H_
