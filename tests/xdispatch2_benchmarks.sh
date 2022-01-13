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
