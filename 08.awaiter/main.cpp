//
// Created by benny on 2022/3/17.
//
#include "Executor.h"
#include "Task.h"
#include "io_utils.h"
#include "Scheduler.h"
#include "Channel.h"
#include "FutureAwaiter.h"

using namespace std::chrono_literals;

struct FakeAwaiter {

  using ResultType = void;
  bool await_ready() { return false; }

  void await_suspend(std::coroutine_handle<> handle) {}

  void await_resume() {}

  void install_executor(AbstractExecutor *) {}
};

Task<void, LooperExecutor> Producer(Channel<int> &channel) {
  int i = 0;
  while (i < 10) {
    debug("send: ", i);
//    co_await channel.write(i++);
    co_await (channel << i++);
//    co_await 300ms;
    co_await SleepAwaiter(300ms);
  }

//  channel.close();
//  debug("close channel, exit.");
}

Task<void, LooperExecutor> Consumer(Channel<int> &channel) {
  while (channel.is_active()) {
    try {
//      auto received = co_await channel.read();
      int received;
      co_await (channel >> received);
      debug("receive: ", received);
      co_await 500ms;
    } catch (std::exception &e) {
      debug("exception: ", e.what());
    }
  }

  debug("exit.");
}

Task<void, LooperExecutor> Consumer2(Channel<int> &channel) {
  while (channel.is_active()) {
    try {
      auto received = co_await channel.read();
      debug("receive2: ", received);
      co_await 200ms;
    } catch (std::exception &e) {
      debug("exception2: ", e.what());
    }
  }

  debug("exit.");
}


void test_channel(Channel<int> &channel) {
  auto producer = Producer(channel);
  auto consumer = Consumer(channel);
  auto consumer2 = Consumer2(channel);

  debug("sleep ...");
  std::this_thread::sleep_for(3s);
  debug("after sleep ...");
}

Task<int, AsyncExecutor> simple_task2() {
  debug("task 2 start ...");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
  debug("task 2 returns after 1s.");
  co_return 2;
}

Task<int, NewThreadExecutor> simple_task3() {
  debug("in task 3 start ...");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  debug("task 3 returns after 2s.");
  co_return 3;
}

template<typename R>
FutureAwaiter<R> as_awaiter(std::future<R> &&future) {
  return FutureAwaiter(std::move(future));
}

Task<int, LooperExecutor> simple_task() {
  debug("task start ...");
  auto result2 = co_await simple_task2().as_awaiter();
  debug("returns from task2: ", result2);
  auto result3 = co_await simple_task3();
  debug("returns from task3: ", result3);
  auto result4 = co_await FutureAwaiter(std::async([]() {
    std::this_thread::sleep_for(1s);
    return 4;
  }));
  debug("returns from task4: ", result4);
  co_return 1 + result2 + result3 + result4;
}

void test_tasks() {
  auto simpleTask = simple_task();
  simpleTask.then([](int i) {
    debug("simple task end: ", i);
  }).catching([](std::exception &e) {
    debug("error occurred", e.what());
  });
  try {
    auto i = simpleTask.get_result();
    debug("simple task end from get: ", i);
  } catch (std::exception &e) {
    debug("error: ", e.what());
  }
}

Task<void> test1() {
//  no matching overloaded function found
//  co_await FakeAwaiter();
  co_return;
}

int main() {
  auto channel = Channel<int>(2);
  test_channel(channel);
  channel.close();
//  test_tasks();
  return 0;
}
