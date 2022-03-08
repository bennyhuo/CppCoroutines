#define __cpp_lib_coroutine

#include <iostream>
#include <coroutine>
#include <future>
#include <chrono>

using namespace std::chrono_literals;

void Fun() {
  std::cout << 1 << std::endl;
  std::cout << 2 << std::endl;
  std::cout << 3 << std::endl;
  std::cout << 4 << std::endl;
}

struct Result {
  struct promise_type {
    std::suspend_never initial_suspend() {
      return {};
    }

    std::suspend_never final_suspend() noexcept {
      return {};
    }

    Result get_return_object() {
      return {};
    }

    void return_void() {

    }

//    void return_value(int value) {
//
//    }

    void unhandled_exception() {

    }
  };
};

struct Awaiter {
  int value;

  bool await_ready() {
    return false;
  }

  void await_suspend(std::coroutine_handle<> coroutine_handle) {
    std::async([=](){
      std::this_thread::sleep_for(1s);
      coroutine_handle.resume();
    });
  }

  int await_resume() {
    return value;
  }
};

Result Coroutine() {
  std::cout << 1 << std::endl;
  std::cout << co_await Awaiter{.value = 1000} << std::endl;
  std::cout << 2 << std::endl;
  std::cout << 3 << std::endl;
  co_await std::suspend_always{};
  std::cout << 4 << std::endl;

  co_return;
};

//Result Coroutine(int start_value) {
//  std::cout << start_value << std::endl;
//  co_await std::suspend_always{};
//  std::cout << start_value + 1 << std::endl;
//
//  co_return 100;
//};

int main() {
  Coroutine();
  return 0;
}
