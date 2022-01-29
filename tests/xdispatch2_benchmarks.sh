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

echo "BENCHMARK SERIAL QUEUES"
echo "======================="
./xdispatch2_testsD -n libdispatch__cxx_benchmark_serial_queue
./xdispatch2_testsD -n naive__cxx_benchmark_serial_queue
./xdispatch2_testsD -n qt5__cxx_benchmark_serial_queue
echo ""

echo "BENCHMARK GLOBAL QUEUES"
echo "======================="
./xdispatch2_testsD -n libdispatch__cxx_benchmark_global_queue
./xdispatch2_testsD -n naive__cxx_benchmark_global_queue
./xdispatch2_testsD -n qt5__cxx_benchmark_global_queue
echo ""

echo "BENCHMARK GROUPS"
echo "================"
./xdispatch2_testsD -n libdispatch__cxx_benchmark_group
./xdispatch2_testsD -n naive__cxx_benchmark_group
./xdispatch2_testsD -n qt5__cxx_benchmark_group
echo ""
