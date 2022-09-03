/*
 * demo.cpp
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

#include <iostream>

#include "xdispatch/dispatch.h"

// Little demo making sure the default use case for a platform
// is supported, i.e. linking against xdispatch only
int
main()
{

    xdispatch::queue custom("platform_demo");

    custom.async([] {
        std::cout << "On the custom queue" << std::endl;

        xdispatch::global_queue().async([] {
            std::cout << "On a global queue" << std::endl;

            xdispatch::main_queue().async([] {
                std::cout << "On the main queue" << std::endl;
                exit(0);
            });
        });
    });

    xdispatch::exec();
    return 1;
}
