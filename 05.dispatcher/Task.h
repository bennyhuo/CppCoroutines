//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_TASK_H_
#define CPPCOROUTINES_04_TASK_TASK_H_

#include "coroutine_common.h"
#include "TaskPromise.h"

template<typename ResultType, typename Executor = NewThreadExecutor>
struct Task {

  using promise_type = TaskPromise<ResultType, Executor>;

  ResultType get_result() {
    return handle.promise().get_result();
  }

  Task &then(std::function<void(ResultType)> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        func(result.get_or_throw());
      } catch (std::exception &e) {
        // ignore.
      }
    });
    return *this;
  }

  Task &catching(std::function<void(std::exception &)> &&func) {
    handle.promise().on_completed([func](auto result) {
      try {
        result.get_or_throw();
      } catch (std::exception &e) {
        func(e);
      }
    });
    return *this;
  }

  Task &finally(std::function<void()> &&func) {
    handle.promise().on_completed([func](auto result) { func(); });
    return *this;
  }

  explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}

  Task(Task &&task) noexcept: handle(std::exchange(task.handle, {})) {}

  Task(Task &) = delete;

  Task &operator=(Task &) = delete;

  ~Task() {
    if (handle) handle.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle;
};

#endif //CPPCOROUTINES_04_TASK_TASK_H_
