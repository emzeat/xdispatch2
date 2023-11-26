#
# conanfile.py
#
# Copyright (c) 2022 - 2023 Marius Zwicker
# All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.files import copy

class XDispatch2Conan(ConanFile):
    name = "xdispatch2"
    license = "Apache 2.0"
    description = "Grand Central Dispatch like C++ threading library built around queues, thread pools and flexible backends."
    url = "https://emzeat.de/xdispatch2"

    short_paths = True

    settings = "os", "compiler", "build_type", "arch"
    options = {
        "build_tests": [True, False],
        "backend_qt5": [True, False],
        "backend_libdispatch": ["None", True, False],
    }
    default_options = {
        "build_tests": False,
        "backend_qt5": True,
        "backend_libdispatch": "None",
    }

    def set_version(self):
        return self.version or '0.0+conan.dev'

    def export_sources(self):
        self.copy("*", excludes=["build/*-*-*", ".conan/*"])

    def requirements(self):
        if self.options.backend_qt5:
            self.requires("qt/5.15.11@emzeat/external")
            self.requires("sqlite3/3.43.1", override=True)
            self.requires("openssl/1.1.1w", override=True)
            self.requires("zlib/1.2.13", override=True)
            self.requires("libpng/1.6.40", override=True)
            if self.settings.os == "Linux":
                self.requires("expat/2.4.8", override=True)
                self.requires("glib/2.78.1", override=True)
                self.requires("libffi/3.4.3", override=True)

    def build_requirements(self):
        self.tool_requires("clang-tools-extra/15.0.7@emzeat/external")
        self.tool_requires("ccache/4.6")
        self.tool_requires("linter-cache/0.2.1@emzeat/oss")

    def generate(self):
        deps = CMakeDeps(self)
        deps.build_context_activated = ["clang-tools-extra", "linter-cache", "ccache"]
        deps.build_context_build_modules = ["clang-tools-extra", "linter-cache", "ccache"]
        deps.generate()

        tc = CMakeToolchain(self)
        tc.user_presets_path = None
        tc.cache_variables["BUILD_XDISPATCH2_BACKEND_QT5"] = self.options.backend_qt5
        if self.options.backend_libdispatch != "None":
            tc.cache_variables["BUILD_XDISPATCH2_BACKEND_LIBDISPATCH"] = self.options.backend_libdispatch
        tc.cache_variables["BUILD_XDISPATCH2_TESTS"] = self.options.build_tests
        tc.cache_variables["MZ_DO_AUTO_FORMAT"] = False
        tc.cache_variables["MZ_DO_CPPLINT"] = False
        tc.cache_variables["MZ_DO_CPPLINT_DIFF"] = False
        tc.cache_variables["BUILD_XDISPATCH2_AS_FRAMEWORK"] = False
        if self.settings.os == "iOS":
            tc.cache_variables["BUILD_XDISPATCH2_STATIC"] = True
        tc.cache_variables["XDISPATCH2_VERSION"] = self.version
        tc.generate()

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(variables={'CONAN_EXPORTED': True})
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst="licenses")

    def _append_implicit_qt5_deps(self, frameworks):
        if self.settings.os in ["iOS"]:
            frameworks.append("CoreFoundation")
            frameworks.append("Security")
            frameworks.append("UIKit")
            frameworks.append("MobileCoreServices")

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "xdispatch2")

        self.cpp_info.components["xdispatch"].names["cmake_target_name"] = "xdispatch2::xdispatch"
        self.cpp_info.components["xdispatch"].libs = ["xdispatch"]
        if self.options.backend_qt5:
            self.cpp_info.components["xdispatch_qt5"].names["cmake_target_name"] = "xdispatch2::xdispatch_qt5"
            self.cpp_info.components["xdispatch_qt5"].libs = ["xdispatch_qt5"]
            self.cpp_info.components["xdispatch_qt5"].requires = ["qt::qtCore", "xdispatch"]
            # these are indirect dependencies inherited through qt5
            self._append_implicit_qt5_deps(self.cpp_info.components["xdispatch_qt5"].frameworks)
            # static builds cannot avoid the Qt5 dependency as symbols cannot be resolved at runtime
            # without requiring users to link the static library in a special way
            if self.settings.os in ["iOS"]:
                self.cpp_info.components["xdispatch"].libs.append("xdispatch_qt5")
                self.cpp_info.components["xdispatch"].requires.append("qt::qtCore")
                self._append_implicit_qt5_deps(self.cpp_info.components["xdispatch"].frameworks)

        self.cpp_info.names["cmake_find_package"] = "xdispatch2"
        self.cpp_info.names["cmake_find_package_multi"] = "xdispatch2"
