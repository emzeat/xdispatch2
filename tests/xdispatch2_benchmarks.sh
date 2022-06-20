#!/bin/sh
#
# xdispatch2_benchmarks.sh
#
# Copyright (c) 2020 - 2022 Marius Zwicker
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

#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "Running in $DIR"
cd $DIR
echo ""

if [ -f xdispatch2_testsD ]; then
    TESTS=./xdispatch2_testsD
elif [ -f xdispatch2_tests ]; then
    TESTS=./xdispatch2_tests
else
    echo "! Failed to find xdispatch2_tests binary, do you need to build it first?"
    exit 1
fi

echo "BENCHMARK SERIAL QUEUES"
echo "======================="
${TESTS} -n libdispatch__cxx_benchmark_serial_queue
${TESTS} -n naive__cxx_benchmark_serial_queue
${TESTS} -n qt5__cxx_benchmark_serial_queue
echo ""

echo "BENCHMARK GLOBAL QUEUES"
echo "======================="
${TESTS} -n libdispatch__cxx_benchmark_global_queue
${TESTS} -n naive__cxx_benchmark_global_queue
${TESTS} -n qt5__cxx_benchmark_global_queue
echo ""

echo "BENCHMARK GROUPS"
echo "================"
${TESTS} -n libdispatch__cxx_benchmark_group
${TESTS} -n naive__cxx_benchmark_group
${TESTS} -n qt5__cxx_benchmark_group
echo ""
