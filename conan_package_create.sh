#!/usr/bin/env bash
#
# conan_package_create.sh
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
if [ -z $PROFILE ]; then
    echo "Please do a local build first to have a suitable profile"
    exit 1
fi

BASE_DIR=$(dirname $PROFILE)
TEST_DIR=$BASE_DIR/conan_package
mkdir -p $TEST_DIR
echo "Using profile '$PROFILE' and testing below '$TEST_DIR'"

cmake -DMZ_SEMVER_TO_FILE=$TEST_DIR/version.txt -P build/semver.cmake
VERSION=$(cat $TEST_DIR/version.txt)
echo "Package export for version '$VERSION'"

set -ex
conan export . xdispatch2/$VERSION@emzeat/oss
