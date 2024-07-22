//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_TASKAWAITER_H_
#define CPPCOROUTINES_04_TASK_TASKAWAITER_H_

#include "coroutine_common.h"
#include "Executor.h"
#include "CommonAwaiter.h"

template<typename ResultType, typename Executor>
struct Task;

template<typename R, typename Executor>
struct TaskAwaiter : public Awaiter<R> {
  explicit TaskAwaiter(Task<R, Executor> &&task) noexcept
      : task(std::move(task)) {}

  TaskAwaiter(TaskAwaiter &&awaiter) noexcept
      : Awaiter<R>(awaiter), task(std::move(awaiter.task)) {}

  TaskAwaiter(TaskAwaiter &) = delete;

  TaskAwaiter &operator=(TaskAwaiter &) = delete;

 protected:
  void after_suspend() override {
    task.finally([this]() {
      this->resume_unsafe();
    });
  }

  void before_resume() override {
    this->_result = Result(task.get_result());
  }

 private:
  Task<R, Executor> task;
};

template<typename Executor>
struct TaskAwaiter<void, Executor> : public Awaiter<void> {
  explicit TaskAwaiter(Task<void, Executor> &&task) noexcept
      : task(std::move(task)) {}

  TaskAwaiter(TaskAwaiter &&awaiter) noexcept
      : Awaiter<void>(awaiter), task(std::move(awaiter.task)) {}

  TaskAwaiter(TaskAwaiter &) = delete;

  TaskAwaiter &operator=(TaskAwaiter &) = delete;

 protected:
  void after_suspend() override {
    task.finally([this]() {
      this->resume_unsafe();
    });
  }

  void before_resume() override {
    task.get_result();
  }

 private:
  Task<void, Executor> task;
};

#endif //CPPCOROUTINES_04_TASK_TASKAWAITER_H_
