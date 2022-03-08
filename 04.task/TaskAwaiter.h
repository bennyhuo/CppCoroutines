//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_TASKAWAITER_H_
#define CPPCOROUTINES_04_TASK_TASKAWAITER_H_

#include "coroutine_common.h"

template<typename ResultType>
struct Task;

template<typename Result>
struct TaskAwaiter {
  explicit TaskAwaiter(Task<Result> &&task) noexcept
      : task(std::move(task)) {}

  TaskAwaiter(TaskAwaiter &&completion) noexcept
      : task(std::exchange(completion.task, {})) {}

  TaskAwaiter(TaskAwaiter &) = delete;

  TaskAwaiter &operator=(TaskAwaiter &) = delete;

  constexpr bool await_ready() const noexcept {
    return false;
  }

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    task.finally([handle]() {
      handle.resume();
    });
  }

  Result await_resume() noexcept {
    return task.get_result();
  }

 private:
  Task<Result> task;
};

#endif //CPPCOROUTINES_04_TASK_TASKAWAITER_H_
