#!/usr/bin/env bash
#
# conan_package_test.sh
#
# Copyright (c) 2022 Marius Zwicker
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

PROFILE=$(ls build/*/profile.conan 2>/dev/null | head -n 1)
BUILD_PROFILE=$(ls build/*/build_profile.conan 2>/dev/null | head -n 1)
if [ -z $PROFILE ]; then
    echo "Please do a local build first to have a suitable profile"
    exit 1
fi

BASE_DIR=$(dirname $PROFILE)
TEST_DIR=$BASE_DIR/conan_package
if [ -z $BUILD_PROFILE ]; then
    PROFILE_ARGS="-pr $PROFILE"
    echo "Using profile '$PROFILE' and testing below '$TEST_DIR'"
else
    PROFILE_ARGS="-pr:h $PROFILE -pr:b $BUILD_PROFILE"
    echo "Cross build with profile '$PROFILE' and testing below '$TEST_DIR'"
fi

cmake -DMZ_SEMVER_TO_FILE=$TEST_DIR/version.txt -P build/semver.cmake
VERSION=$(cat $TEST_DIR/version.txt)
echo "Test build for version '$VERSION'"

#rm -rf $TEST_DIR/source
#mkdir -p $TEST_DIR/source

mkdir -p $TEST_DIR/build

mkdir -p $TEST_DIR/install

mkdir -p $TEST_DIR/package

set -ex
conan source -sf $TEST_DIR/source .
conan install -if $TEST_DIR/install $PROFILE_ARGS conanfile.py
conan build -bf $TEST_DIR/build -if $TEST_DIR/install -pf $TEST_DIR/package -sf $TEST_DIR/source .
conan package -bf $TEST_DIR/build -if $TEST_DIR/install -pf $TEST_DIR/package -sf $TEST_DIR/source .
