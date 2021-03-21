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

#ifndef XDISPATCH_LIBDISPATCH_EXECUTION_H_
#define XDISPATCH_LIBDISPATCH_EXECUTION_H_

#include "xdispatch/dispatch.h"

extern "C"
{
    void _xdispatch2_run_wrap(void*);

    void _xdispatch2_run_wrap_delete(void*);

    void _xdispatch2_run_iter_wrap(void*, size_t);
}

__XDISPATCH_BEGIN_NAMESPACE
namespace libdispatch {

template<class T>
class wrap_T
{
public:
    explicit wrap_T(const std::shared_ptr<T>& t)
      : m_type(t)
    {}

    ~wrap_T() = default;

    inline const std::shared_ptr<T>& type() const { return m_type; }

private:
    const std::shared_ptr<T> m_type;
};

using operation_wrap = wrap_T<operation>;
using iteration_operation_wrap = wrap_T<iteration_operation>;

} // namespace libdispatch
__XDISPATCH_END_NAMESPACE

#endif /* XDISPATCH_LIBDISPATCH_EXECUTION_H_ */
