"""
 conanfile.py

 Copyright (c) 2022 Marius Zwicker
 All rights reserved.

 SPDX-License-Identifier: Apache-2.0

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

from conans import ConanFile

class XDispatch2Conan(ConanFile):
    generators = "cmake_find_package", "cmake_paths", "json"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("qt/5.15.4@emzeat/external")
        # pin versions of dependent packages
        self.requires("sqlite3/3.29.0")
        if self.settings.os == "Linux":
            self.requires("xorg/system@emzeat/external")
            self.requires("expat/2.4.8")
            self.requires("libiconv/1.17")
            self.requires("pcre2/10.39")
            self.requires("glib/2.72.0")

    def build_requirements(self):
        self.tool_requires("clang-tools-extra/13.0.1@emzeat/external")

    def imports(self):
        self.copy("*.dll", dst="${EXECUTABLE_OUTPUT_PATH}", src="bin")
        self.copy("*.dylib*", dst="${EXECUTABLE_OUTPUT_PATH}", src="lib")
        self.copy("*.so*", dst="${EXECUTABLE_OUTPUT_PATH}", src="lib")
        self.copy("license*", dst="${EXECUTABLE_OUTPUT_PATH}/3rdparty", folder=True, ignore_case=True)
