/*
 * operation.h
 *
 * Copyright (c) 2011 - 2022 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 */

#ifndef XDISPATCH_OPERATION_H_
#define XDISPATCH_OPERATION_H_

/**
 * @addtogroup xdispatch
 * @{
 */

#include "dispatch_decl.h"

#include <string>
#include <memory>
#include <atomic>

__XDISPATCH_BEGIN_NAMESPACE

class operation;
template<typename... Params>
class parameterized_operation;

using operation_ptr = std::shared_ptr<operation>;
template<typename... Params>
using parameterized_operation_ptr =
  std::shared_ptr<parameterized_operation<Params...>>;

class queued_operation;
template<typename... Params>
using queued_parameterized_operation = parameterized_operation_ptr<Params...>;

/**
  Will synchronously execute the given operation on the current thread
  */
XDISPATCH_EXPORT void
execute_operation_on_this_thread(const operation_ptr&);

/**
  Will synchronously execute the given operation on the current thread
  for the given parameters
  */
template<typename... Params>
XDISPATCH_EXPORT void
execute_operation_on_this_thread(const parameterized_operation_ptr<Params...>&,
                                 Params... params);

/**
  Private Internal Functions
*/
template<typename... Params>
queued_parameterized_operation<Params...>
queue_operation_with_target(const parameterized_operation_ptr<Params...>&,
                            void*);
template<typename... Params>
void
execute_operation_on_this_thread(
  const queued_parameterized_operation<Params...>&,
  Params... params);

/**
   @brief Common base class shared with all operations
 */
class XDISPATCH_EXPORT base_operation
{
public:
    base_operation(const base_operation& other) = delete;

protected:
    base_operation() = default;
    ~base_operation() = default;
};

/**
  An operation is a functor used to
  define single portions of work to be
  dispatched to a single queue.

  Derive from this class and implement
  the operator to create specific operations
  that can be executed on a queue.
  */
class XDISPATCH_EXPORT operation : public base_operation
{
public:
    operation() = default;

    virtual ~operation() = default;

protected:
    /**
        Will be invoked when the operation gets executed
        after it had been queued.
     */
    virtual void operator()() = 0;

private:
    // allow access to internals
    friend queued_operation queue_operation_with_target(const operation_ptr&,
                                                        void*);
    friend void execute_operation_on_this_thread(const queued_operation&);
};

template<typename Func, typename... Params>
class function_parameterized_operation;

template<class T, typename... Params>
class member_parameterized_operation;

/**
  @see operation

  Same as operation except that a parameter
  will be passed whenever this
  functor is executed on a queue.
  */
template<typename... Params>
class parameterized_operation : public base_operation
{
public:
    parameterized_operation() = default;

    virtual ~parameterized_operation() = default;

    inline static parameterized_operation_ptr<Params...> make(
      const parameterized_operation_ptr<Params...>& op)
    {
        return op;
    }

    template<typename Func>
    inline static typename std::enable_if<
      !std::is_convertible<Func, parameterized_operation_ptr<Params...>>::value,
      parameterized_operation_ptr<Params...>>::type
    make(const Func& f)
    {
        return std::make_shared<
          function_parameterized_operation<Func, Params...>>(f);
    }

    template<class T>
    inline static parameterized_operation_ptr<Params...> make(
      T* object,
      void (T::*function)(Params...))
    {
        return std::make_shared<member_parameterized_operation<T, Params...>>(
          object, function);
    }

protected:
    /**
        Will be invoked when the operation gets executed
        after it had been queued.

        @param index The iterator position for which
                     the operation is executed
     */
    virtual void operator()(Params... params) = 0;

private:
    // allow access to internals
    friend queued_parameterized_operation<Params...>
    queue_operation_with_target<Params...>(
      const parameterized_operation_ptr<Params...>&,
      void*);
    friend void execute_operation_on_this_thread<Params...>(
      const queued_parameterized_operation<Params...>&,
      Params...);
};

/**
  Same as operation except that an
  index will be passed whenever this
  functor is executed on a queue.
*/
using iteration_operation = parameterized_operation<size_t>;

using iteration_operation_ptr = std::shared_ptr<iteration_operation>;

/**
  A simple operation for wrapping the given
  function as an xdispatch::operation
  */
template<typename Func>
class function_operation : public operation
{
public:
    function_operation(const Func& b)
      : operation()
      , m_function(b)
    {}

    function_operation(const function_operation& other) = default;

    ~function_operation() override = default;

    void operator()() final { m_function(); }

private:
    const Func m_function;
};

/**
  Provides a template functor to wrap
  a function pointer to a memberfunction of an object as operation
  */
template<class T>
class member_operation : public operation
{
public:
    member_operation(T* object, void (T::*function)())
      : operation()
      , m_obj(object)
      , m_func(function)
    {}

    void operator()() final { (*m_obj.*m_func)(); }

private:
    T* const m_obj;
    void (T::*m_func)();
};

inline operation_ptr
make_operation(const operation_ptr& op)
{
    return op;
}

template<typename Func>
inline typename std::enable_if<!std::is_convertible<Func, operation_ptr>::value,
                               operation_ptr>::type
make_operation(const Func& f)
{
    return std::make_shared<function_operation<Func>>(f);
}

template<class T>
inline operation_ptr
make_operation(T* object, void (T::*function)())
{
    return std::make_shared<member_operation<T>>(object, function);
}

/**
  A simple parameterized operation needed when
  applying a function object several times
  */
template<typename Func, typename... Params>
class function_parameterized_operation
  : public parameterized_operation<Params...>
{
public:
    function_parameterized_operation(const Func b)
      : parameterized_operation<Params...>()
      , m_function(b)
    {}

    function_parameterized_operation(
      const function_parameterized_operation& other) = default;

    ~function_parameterized_operation() override = default;

    void operator()(Params... params) final { m_function(params...); }

private:
    const Func m_function;
};

template<typename Func>
using function_iteration_operation = parameterized_operation<Func, size_t>;

/**
  Provides a template functor to wrap
  a function pointer to a memberfunction of an object as parameterized_operation
  */
template<class T, typename... Params>
class member_parameterized_operation : public parameterized_operation<Params...>
{
public:
    member_parameterized_operation(T* object, void (T::*function)(Params...))
      : parameterized_operation<Params...>()
      , m_obj(object)
      , m_func(function)
    {}

    void operator()(Params... params) final { (*m_obj.*m_func)(params...); }

private:
    T* const m_obj;
    void (T::*m_func)(Params...);
};

template<class T>
using member_iteration_operation = member_parameterized_operation<T, size_t>;

inline std::shared_ptr<iteration_operation>
make_iteration_operation(const std::shared_ptr<iteration_operation>& op)
{
    return iteration_operation::make(op);
}

template<typename Func>
inline typename std::enable_if<
  !std::is_convertible<Func, std::shared_ptr<iteration_operation>>::value,
  std::shared_ptr<iteration_operation>>::type
make_iteration_operation(const Func& f)
{
    return iteration_operation::make(f);
}

template<class T>
inline std::shared_ptr<iteration_operation>
make_iteration_operation(T* object, void (T::*function)(size_t))
{
    return iteration_operation::make(object, function);
}

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_OPERATION_H_ */
