//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_EXECUTOR_H_
#define CPPCOROUTINES_04_TASK_EXECUTOR_H_

#include <queue>
#include <mutex>
#include <functional>
#include <future>
#include <map>
#include "io_utils.h"

class AbstractExecutor {
 public:
  virtual void execute(std::function<void()> &&func) = 0;
};

class NoopExecutor : public AbstractExecutor {
 public:
  void execute(std::function<void()> &&func) override {
    func();
  }
};

class NewThreadExecutor : public AbstractExecutor {
 public:
  void execute(std::function<void()> &&func) override {
    std::thread(func).detach();
  }
};

class AsyncExecutor : public AbstractExecutor {
 public:
  void execute(std::function<void()> &&func) override {
    std::unique_lock lock(future_lock);
    auto id = nextId++;
    lock.unlock();

    auto future = std::async([this, id, func](){
      func();

      std::unique_lock lock(future_lock);
      // move future to stack so that it will be destroyed after unlocked.
      auto f = std::move(this->futures[id]);
      this->futures.erase(id);
      lock.unlock();
    });

    lock.lock();
    this->futures[id] = std::move(future);
    lock.unlock();
  }
 private:
  std::mutex future_lock;
  int nextId = 0;
  std::map<int, std::future<void>> futures{};
};

class LooperExecutor : public AbstractExecutor {
 private:
  std::condition_variable queue_condition;
  std::mutex queue_lock;
  std::queue<std::function<void()>> executable_queue;

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
      auto func = executable_queue.front();
      executable_queue.pop();
      lock.unlock();

      func();
    }
    debug("run_loop exit.");
  }

 public:

  LooperExecutor() {
    is_active.store(true, std::memory_order_relaxed);
    work_thread = std::thread(&LooperExecutor::run_loop, this);
  }

  ~LooperExecutor() {
    shutdown(false);
    if (work_thread.joinable()) {
      work_thread.join();
    }
  }

  void execute(std::function<void()> &&func) override {
    std::unique_lock lock(queue_lock);
    if (is_active.load(std::memory_order_relaxed)) {
      executable_queue.push(func);
      lock.unlock();
      queue_condition.notify_one();
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
};

class SharedLooperExecutor : public AbstractExecutor {
 public:
  void execute(std::function<void()> &&func) override {
    static LooperExecutor sharedLooperExecutor;
    sharedLooperExecutor.execute(std::move(func));
  }
};

#endif //CPPCOROUTINES_04_TASK_EXECUTOR_H_
