//
// Created by benny on 2022/3/18.
//

#ifndef CPPCOROUTINES_TASKS_04_TASK_SCHEDULER_H_
#define CPPCOROUTINES_TASKS_04_TASK_SCHEDULER_H_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>

#include "io_utils.h"

class DelayedExecutable {
 public:
  DelayedExecutable(std::function<void()> &&func, long long delay) : func(std::move(func)) {
    using namespace std;
    using namespace std::chrono;
    auto now = system_clock::now();
    auto current = duration_cast<milliseconds>(now.time_since_epoch()).count();

    scheduled_time = current + delay;
  }

  long long delay() const {
    using namespace std;
    using namespace std::chrono;

    auto now = system_clock::now();
    auto current = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return scheduled_time - current;
  }

  long long get_scheduled_time() const {
    return scheduled_time;
  }

  void operator()() {
    func();
  }

 private:
  long long scheduled_time;
  std::function<void()> func;
};

class DelayedExecutableCompare {
 public:
  bool operator()(DelayedExecutable &left, DelayedExecutable &right) {
    return left.get_scheduled_time() > right.get_scheduled_time();
  }
};

class Scheduler {
 private:
  std::condition_variable queue_condition;
  std::mutex queue_lock;
  std::priority_queue<DelayedExecutable, std::vector<DelayedExecutable>, DelayedExecutableCompare> executable_queue;

  std::atomic<bool> is_active;
  std::thread work_thread;

  void run_loop() {
    while (is_active.load(std::memory_order_relaxed) || !executable_queue.empty()) {
      std::unique_lock lock(queue_lock);
      if (executable_queue.empty()) {
        queue_condition.wait(lock);
        if (executable_queue.empty()) {
          continue;
        }
      }
      auto executable = executable_queue.top();
      long long delay = executable.delay();
      if (delay > 0) {
        auto status = queue_condition.wait_for(lock, std::chrono::milliseconds(delay));
        if (status != std::cv_status::timeout) {
          // a new executable should be executed before.
          continue;
        }
      }
      executable_queue.pop();
      lock.unlock();
      executable();
    }
    debug("run_loop exit.");
  }
 public:

  Scheduler() {
    is_active.store(true, std::memory_order_relaxed);
    work_thread = std::thread(&Scheduler::run_loop, this);
  }

  ~Scheduler() {
    shutdown(false);
    join();
  }

  void execute(std::function<void()> &&func, long long delay) {
    delay = delay < 0 ? 0 : delay;
    std::unique_lock lock(queue_lock);
    if (is_active.load(std::memory_order_relaxed)) {
      bool need_notify = executable_queue.empty() || executable_queue.top().delay() > delay;
      executable_queue.push(DelayedExecutable(std::move(func), delay));
      lock.unlock();
      if (need_notify) {
        queue_condition.notify_one();
      }
    }
  }

  void shutdown(bool wait_for_complete = true) {
    is_active.store(false, std::memory_order_relaxed);
    if (!wait_for_complete) {
      // clear queue.
      std::unique_lock lock(queue_lock);
      decltype(executable_queue) empty_queue;
      std::swap(executable_queue, empty_queue);
      lock.unlock();
    }

    queue_condition.notify_all();
  }

  void join() {
    if (work_thread.joinable()) {
      work_thread.join();
    }
  }
};

#endif //CPPCOROUTINES_TASKS_04_TASK_SCHEDULER_H_
