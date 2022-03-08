//
// Created by benny on 2022/3/20.
//

#ifndef CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_
#define CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_

#include "Executor.h"
#include "Scheduler.h"
#include "coroutine_common.h"

struct SleepAwaiter {

  explicit SleepAwaiter(AbstractExecutor *executor, long long duration) noexcept
      : _executor(executor), _duration(duration) {}

  bool await_ready() const { return false; }

  void await_suspend(std::coroutine_handle<> handle) const {
    static Scheduler scheduler;

    scheduler.execute([this, handle]() {
      _executor->execute([handle]() {
        handle.resume();
      });
    }, _duration);
  }

  void await_resume() {}

 private:
  AbstractExecutor *_executor;
  long long _duration;
};

#endif //CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_
