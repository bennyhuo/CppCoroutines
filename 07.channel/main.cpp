//
// Created by benny on 2022/3/17.
//
#include "Executor.h"
#include "Task.h"
#include "io_utils.h"
#include "Scheduler.h"
#include "Channel.h"

using namespace std::chrono_literals;

Task<void, LooperExecutor> Producer(Channel<int> &channel) {
  int i = 0;
  while (i < 10) {
    debug("send: ", i);
//    co_await channel.write(i++);
    co_await (channel << i++);
    co_await 50ms;
  }

  co_await 5s;
  channel.close();
  debug("close channel, exit.");
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
      co_await 300ms;
    } catch (std::exception &e) {
      debug("exception2: ", e.what());
    }
  }

  debug("exit.");
}

void test_channel() {
  auto channel = Channel<int>(5);
  auto producer = Producer(channel);
  auto consumer = Consumer(channel);
  auto consumer2 = Consumer2(channel);

  std::this_thread::sleep_for(10s);
}

int main() {
  test_channel();
  return 0;
}
