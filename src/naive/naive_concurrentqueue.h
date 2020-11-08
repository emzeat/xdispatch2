/*
* Copyright (c) 2011-2013 MLBA-Team. All rights reserved.
*
* @MLBA_OPEN_LICENSE_HEADER_START@
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
*
* @MLBA_OPEN_LICENSE_HEADER_END@
*/

#include "3rdparty/concurrentqueue.h"

#ifndef XDISPATCH_NAIVE_CONCURRENT_QUEUE_H_
#define XDISPATCH_NAIVE_CONCURRENT_QUEUE_H_

__XDISPATCH_BEGIN_NAMESPACE
namespace naive
{

/**
  Make use of the exceptional lockfree queue handling
  multiple producers and consumers which can be found at

  https://github.com/cameron314/concurrentqueue
*/
template<typename T>
using concurrentqueue = ::moodycamel::ConcurrentQueue<T>;

}
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_NAIVE_CONCURRENT_QUEUE_H_ */
