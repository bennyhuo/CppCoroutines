//
// Created by benny on 2022/3/17.
//
#include "Executor.h"
#include "Task.h"
#include "io_utils.h"
#include "Scheduler.h"

void test_scheduler() {
  auto scheduler = Scheduler();

  debug("start");
  scheduler.execute([]() { debug("2"); }, 100);
  scheduler.execute([]() { debug("1"); }, 50);
  scheduler.execute([]() { debug("6"); }, 1000);
  scheduler.execute([]() { debug("5"); }, 500);
  scheduler.execute([]() { debug("3"); }, 200);
  scheduler.execute([]() { debug("4"); }, 300);

  scheduler.shutdown();
  scheduler.join();
}

Task<int, AsyncExecutor> simple_task2() {
  debug("task 2 start ...");
  using namespace std::chrono_literals;
  co_await 1s;
  debug("task 2 returns after 1s.");
  co_return 2;
}

Task<int, NewThreadExecutor> simple_task3() {
  debug("in task 3 start ...");
  using namespace std::chrono_literals;
  co_await 2s;
  debug("task 3 returns after 2s.");
  co_return 3;
}

Task<int, LooperExecutor> simple_task() {
  debug("task start ...");
  using namespace std::chrono_literals;
  co_await 100ms;
  debug("after 100ms ...");
  auto result2 = co_await simple_task2();
  debug("returns from task2: ", result2);

  co_await 500ms;
  debug("after 500ms ...");
  auto result3 = co_await simple_task3();
  debug("returns from task3: ", result3);
  co_return 1 + result2 + result3;
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

int main() {
  test_tasks();
//  test_scheduler();
  return 0;
}
