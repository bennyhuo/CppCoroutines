//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_TASKS_04_TASK_TASKPROMISE_H_
#define CPPCOROUTINES_TASKS_04_TASK_TASKPROMISE_H_

#include <functional>
#include <mutex>
#include <list>
#include <optional>

#include "coroutine_common.h"
#include "Result.h"
#include "DispatchAwaiter.h"
#include "TaskAwaiter.h"
#include "SleepAwaiter.h"
#include "ChannelAwaiter.h"
#include "CommonAwaiter.h"

template<typename AwaiterImpl, typename R>
concept AwaiterImplRestriction = std::is_base_of<Awaiter<R>, AwaiterImpl>::value;

template<typename ResultType, typename Executor>
class Task;

template<typename ResultType, typename Executor>
struct TaskPromise {
  DispatchAwaiter initial_suspend() { return DispatchAwaiter{&executor}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  Task<ResultType, Executor> get_return_object() {
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
  }

  template<typename _ResultType, typename _Executor>
  TaskAwaiter<_ResultType, _Executor> await_transform(Task<_ResultType, _Executor> &&task) {
    return await_transform(TaskAwaiter<_ResultType, _Executor>(std::move(task)));
  }

  template<typename _Rep, typename _Period>
  auto await_transform(std::chrono::duration<_Rep, _Period> &&duration) {
    return await_transform(SleepAwaiter(duration));
  }

  template<typename AwaiterImpl>
  requires AwaiterImplRestriction<AwaiterImpl, typename AwaiterImpl::ResultType>
  AwaiterImpl await_transform(AwaiterImpl awaiter) {
    awaiter.install_executor(&executor);
    return awaiter;
  }

  void unhandled_exception() {
    std::lock_guard lock(completion_lock);
    result = Result<ResultType>(std::current_exception());
    completion.notify_all();
    notify_callbacks();
  }

  void return_value(ResultType value) {
    std::lock_guard lock(completion_lock);
    result = Result<ResultType>(std::move(value));
    completion.notify_all();
    notify_callbacks();
  }

  ResultType get_result() {
    // blocking for result or throw on exception
    std::unique_lock lock(completion_lock);
    if (!result.has_value()) {
      completion.wait(lock);
    }
    return result->get_or_throw();
  }

  void on_completed(std::function<void(Result<ResultType>)> &&func) {
    std::unique_lock lock(completion_lock);
    if (result.has_value()) {
      auto value = result.value();
      lock.unlock();
      func(value);
    } else {
      completion_callbacks.push_back(func);
    }
  }

 private:
  std::optional<Result<ResultType>> result;

  std::mutex completion_lock;
  std::condition_variable completion;

  std::list<std::function<void(Result<ResultType>)>> completion_callbacks;

  Executor executor;

  void notify_callbacks() {
    auto value = result.value();
    for (auto &callback : completion_callbacks) {
      callback(value);
    }
    completion_callbacks.clear();
  }

};

template<typename Executor>
struct TaskPromise<void, Executor> {
  DispatchAwaiter initial_suspend() { return DispatchAwaiter{&executor}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  Task<void, Executor> get_return_object() {
    return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
  }

  template<typename _ResultType, typename _Executor>
  TaskAwaiter<_ResultType, _Executor> await_transform(Task<_ResultType, _Executor> &&task) {
    return await_transform(TaskAwaiter<_ResultType, _Executor>(std::move(task)));
  }

  template<typename _Rep, typename _Period>
  SleepAwaiter await_transform(std::chrono::duration<_Rep, _Period> &&duration) {
    return await_transform(SleepAwaiter(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
  }

  template<typename AwaiterImpl>
  requires AwaiterImplRestriction<AwaiterImpl, typename AwaiterImpl::ResultType>
  AwaiterImpl await_transform(AwaiterImpl &&awaiter) {
    awaiter.install_executor(&executor);
    return awaiter;
  }

  void get_result() {
    // blocking for result or throw on exception
    std::unique_lock lock(completion_lock);
    if (!result.has_value()) {
      completion.wait(lock);
    }
    result->get_or_throw();
  }

  void unhandled_exception() {
    std::lock_guard lock(completion_lock);
    result = Result<void>(std::current_exception());
    completion.notify_all();
    notify_callbacks();
  }

  void return_void() {
    std::lock_guard lock(completion_lock);
    result = Result<void>();
    completion.notify_all();
    notify_callbacks();
  }

  void on_completed(std::function<void(Result<void>)> &&func) {
    std::unique_lock lock(completion_lock);
    if (result.has_value()) {
      auto value = result.value();
      lock.unlock();
      func(value);
    } else {
      completion_callbacks.push_back(func);
    }
  }

 private:
  std::optional<Result<void>> result;

  std::mutex completion_lock;
  std::condition_variable completion;

  std::list<std::function<void(Result<void>)>> completion_callbacks;

  Executor executor;

  void notify_callbacks() {
    auto value = result.value();
    for (auto &callback : completion_callbacks) {
      callback(value);
    }
    completion_callbacks.clear();
  }

};

#endif //CPPCOROUTINES_TASKS_04_TASK_TASKPROMISE_H_
