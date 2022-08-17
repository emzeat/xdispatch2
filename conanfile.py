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

from conans import ConanFile, CMake

class XDispatch2Conan(ConanFile):
    name = "xdispatch2"
    license = "Apache 2.0"
    description = "Grand Central Dispatch like C++ threading library built around queues, thread pools and flexible backends."
    url = "https://emzeat.de/xdispatch2"

    short_paths = True
    generators = "cmake_find_package", "cmake_paths", "json"

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

    def export_sources(self):
        self.copy("*", excludes=["build/*-*-*"])

    def requirements(self):
        if self.options.backend_qt5:
            self.requires("qt/5.15.4@emzeat/external")
            self.requires("sqlite3/3.29.0", override=True)
            if self.settings.os == "Linux":
                self.requires("xorg/system@emzeat/external", override=True)

    def build_requirements(self):
        self.tool_requires("clang-tools-extra/13.0.1@emzeat/external")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_MODULE_PATH"] = self.build_folder.replace("\\", "/")
        cmake.definitions["CONAN_EXPORTED"] = True
        cmake.definitions["BUILD_XDISPATCH2_BACKEND_QT5"] = self.options.backend_qt5
        if self.options.backend_libdispatch != "None":
            cmake.definitions["BUILD_XDISPATCH2_BACKEND_LIBDISPATCH"] = self.options.backend_libdispatch
        cmake.definitions["BUILD_XDISPATCH2_TESTS"] = self.options.build_tests
        cmake.definitions["MZ_DO_AUTO_FORMAT"] = False
        cmake.definitions["MZ_DO_CPPLINT"] = False
        cmake.definitions["MZ_DO_CPPLINT_DIFF"] = False
        cmake.definitions["MZ_CONAN_INSTALL_DIR"] = self.install_folder
        cmake.definitions["BUILD_XDISPATCH2_AS_FRAMEWORK"] = False
        cmake.definitions["XDISPATCH2_VERSION"] = self.version or '0.0+conan.dev'
        cmake.configure(source_folder=self.source_folder,
                        build_folder=self.build_folder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="${EXECUTABLE_OUTPUT_PATH}", src="bin")
        self.copy("*.dylib*", dst="${EXECUTABLE_OUTPUT_PATH}", src="lib")
        self.copy("*.so*", dst="${EXECUTABLE_OUTPUT_PATH}", src="lib")
        self.copy("license*", dst="${EXECUTABLE_OUTPUT_PATH}/3rdparty", folder=True, ignore_case=True)

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
        self.copy("LICENSE", src=self.source_folder, dst="licenses")

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "xdispatch2")
        self.cpp_info.set_property("cmake_target_name", "xdispatch2::xdispatch2")
        self.cpp_info.libs = ["xdispatch"]
        if self.options.backend_qt5:
            self.cpp_info.requires = ["qt::qtCore"]
            if self.settings.os in ["iOS"]:
                # these are indirect dependencies inherited through qt5
                self.cpp_info.frameworks.append("CoreFoundation")
                self.cpp_info.frameworks.append("Security")
                self.cpp_info.frameworks.append("UIKit")
                self.cpp_info.frameworks.append("MobileCoreServices")

        self.cpp_info.names["cmake_find_package"] = "xdispatch2"
        self.cpp_info.names["cmake_find_package_multi"] = "xdispatch2"
