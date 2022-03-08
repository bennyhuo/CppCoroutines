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

  volatile bool is_active = true;
  std::thread work_thread;
 public:

  Scheduler() {
    work_thread = std::thread([this]() {
      while (is_active || !executable_queue.empty()) {
        std::unique_lock lock(queue_lock);
        if (executable_queue.empty()) {
          queue_condition.wait(lock);
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
    });
  }

  ~Scheduler() {
    shutdown(false);
  }

  void execute(std::function<void()> &&func, long delay) {
    std::lock_guard lock(queue_lock);
    if (is_active) {
      executable_queue.push(DelayedExecutable(std::move(func), delay));
      queue_condition.notify_one();
    }
  }

  void shutdown(bool wait_for_complete = true) {
    is_active = false;
    if (wait_for_complete) {
      if (work_thread.joinable()) {
        work_thread.join();
      }
    } else {
      std::lock_guard lock(queue_lock);
      // clear queue.
      decltype(executable_queue) empty_queue;
      std::swap(executable_queue, empty_queue);
      if (work_thread.joinable()) {
        work_thread.detach();
      }
    }
  }

};

#endif //CPPCOROUTINES_TASKS_04_TASK_SCHEDULER_H_
