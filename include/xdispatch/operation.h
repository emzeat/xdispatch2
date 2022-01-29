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

#include "platform.h"
#include "dispatch_decl.h"

#include <string>
#include <memory>

__XDISPATCH_BEGIN_NAMESPACE

/**
  An operation is a functor used to
  define single portions of work to be
  dispatched to a single queue.

  Derive from this class and implement
  the operator to create specific operations
  that can be executed on a queue.
  */
class XDISPATCH_EXPORT operation
{
public:
    operation();
    operation(const operation& other) = delete;

    virtual ~operation() = default;

protected:
    /**
        Will be invoked when the operation gets executed
        after it had been queued.
     */
    virtual void operator()() = 0;

private:
    // opaque data pointer for internal book keeping
    void* m_d;
    friend void queue_operation_with_d(operation&, void*);
    friend XDISPATCH_EXPORT void execute_operation_on_this_thread(operation&);
};

using operation_ptr = std::shared_ptr<operation>;

/**
  Will synchronously execute the given operation on the current thread
  */
XDISPATCH_EXPORT void
execute_operation_on_this_thread(operation&);

/**
  @see operation

  Same as operation except that an
  index will be passed whenever this
  functor is executed on a queue.
  */
class XDISPATCH_EXPORT iteration_operation
{
public:
    iteration_operation();

    virtual ~iteration_operation() = default;

protected:
    /**
        Will be invoked when the operation gets executed
        after it had been queued.

        @param index The iterator position for which
                     the operation is executed
     */
    virtual void operator()(size_t index) = 0;

private:
    // opaque data pointer for internal book keeping
    void* m_d;
    friend void queue_operation_with_d(iteration_operation&, void*);
    friend XDISPATCH_EXPORT void execute_operation_on_this_thread(
      iteration_operation&,
      size_t);
};

using iteration_operation_ptr = std::shared_ptr<iteration_operation>;

/**
  Will synchronously execute the given operation on the current thread
  for the given iteration
  */
XDISPATCH_EXPORT void
execute_operation_on_this_thread(iteration_operation&, size_t);

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
  A simple iteration operation needed when
  applying a function object several times
  */
template<typename Func>
class function_iteration_operation : public iteration_operation
{
public:
    function_iteration_operation(const Func b)
      : iteration_operation()
      , m_function(b)
    {}

    function_iteration_operation(const function_iteration_operation& other) =
      default;

    ~function_iteration_operation() override = default;

    void operator()(size_t index) final { m_function(index); }

private:
    const Func m_function;
};

/**
  Provides a template functor to wrap
  a function pointer to a memberfunction of an object as iteration_operation
  */
template<class T>
class member_iteration_operation : public iteration_operation
{
public:
    member_iteration_operation(T* object, void (T::*function)(size_t))
      : iteration_operation()
      , m_obj(object)
      , m_func(function)
    {}

    void operator()(size_t index) final { (*m_obj.*m_func)(index); }

private:
    T* const m_obj;
    void (T::*m_func)(size_t);
};

inline iteration_operation_ptr
make_iteration_operation(const iteration_operation_ptr& op)
{
    return op;
}

template<typename Func>
inline typename std::enable_if<
  !std::is_convertible<Func, iteration_operation_ptr>::value,
  iteration_operation_ptr>::type
make_iteration_operation(const Func& f)
{
    return std::make_shared<function_iteration_operation<Func>>(f);
}

template<class T>
inline iteration_operation_ptr
make_iteration_operation(T* object, void (T::*function)(size_t))
{
    return std::make_shared<member_iteration_operation<T>>(object, function);
}

__XDISPATCH_END_NAMESPACE

/** @} */

#endif /* XDISPATCH_OPERATION_H_ */
