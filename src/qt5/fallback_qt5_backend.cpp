/*
 * fallback_qt5_backend.cpp
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

#define AVOID_QT5_BACKEND
#include "qt5_backend_internal.h"

// This is defining a backend created via the qt5 entrypoints
// but not actually introducing any dependency on the qt5 libs
XDISPATCH_DEFINE_BACKEND(qt5, xdispatch::qt5::backend_base)

/*
    Some background on why this is required:

    Optional backends work by declaring the backend and a fallback in backend.cpp
    which will ultimately try to resolve the backend entrypoint declared via
    `XDISPATCH_DEFINED_BACKEND` at runtime and if not found proceed to using the
    fallback backend. This works reliably for shared libraries but does not work
    when linking statically. In the latter case the linker is free to not include
    any unused symbols at link time at all.

    While there is workarounds available, these are highly platform and linker
    specific and require the user of the static libraries to use specific options
    when linking the final executable which would expose implementation detail
    of xdispatch.

    Mechanisms such as weak linking to allow for optional symbols to be
    overridden at link time also do not work with static libraries but best for
    shared or object libraries.

    Solutions tried but ultimately discarded:
    * Building duplicate versions of xdispatch by use of proxy libraries where
      either xdispatch or xdispatch_qt5 can be linked to get the respective
      set of backends with the former including all backends except qt5 and the
      latter including all backends. The problem found with this solution is
      that no warning will be raised when linking both, while availability of
      the qt5 backend would depend on link order (as both library versions
      would define the `backend_for_type` table - one with, the other without qt5)
    * Testing `dlsym` with static libraries by forcing symbols to get exported,
      this proved to be heavily platform and linker specific. Most importantly
      this requires cooperation of the executable creator which was discarded
      for the reasons already listed above.
    * Not providing any optional backends for static builds at all. This would
      make a build truly static in that if qt5 was enabled in a static build, the
      dependency would always be there. This was discarded for a lack of flexibility
      as a) the dependency would become implicit and b) no mixed use is available
      anymore.
    * The finally chosen solution is to keep the separation for optional backends
      and allowing to link with dummy implementations in the static case to
      explicitly avoid the dependency despite being configured.
*/
