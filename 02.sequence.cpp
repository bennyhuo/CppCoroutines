//
// Created by benny on 2022/1/31.
//
#define __cpp_lib_coroutine
#include <coroutine>
#include <exception>
#include <iostream>
#include <thread>

struct Generator {

  class ExhaustedException: std::exception { };

  struct promise_type {
    int value;
    bool is_ready = false;

    std::suspend_never initial_suspend() { return {}; };

    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always await_transform(int value) {
      this->value = value;
      is_ready = true;
      return {};
    }

    void unhandled_exception() {

    }

    Generator get_return_object() {
      return Generator{ std::coroutine_handle<promise_type>::from_promise(*this) };
    }

    void return_void() { }
  };

  std::coroutine_handle<promise_type> handle;

  bool has_next() {
    if (!handle || handle.done()) {
      return false;
    }

    if (!handle.promise().is_ready) {
      handle.resume();
    }

    if (handle.done()) {
      return false;
    } else {
      return true;
    }
  }

  int next() {
    if (has_next()) {
      handle.promise().is_ready = false;
      return handle.promise().value;
    }
    throw ExhaustedException();
  }

  explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
      : handle(handle) {}

  Generator(Generator &&generator) noexcept
      : handle(std::exchange(generator.handle, {})) {}

  Generator(Generator &) = delete;
  Generator &operator=(Generator &) = delete;

  ~Generator() {
    if (handle) handle.destroy();
  }
};

Generator sequence() {
  int i = 0;
  while (i < 5) {
    co_await i++;
  }
}

Generator returns_generator() {
  auto g = sequence();
  if (g.has_next()) {
    std::cout << g.next() << std::endl;
  }
  return g;
}

int main() {
  auto generator = returns_generator();
  for (int i = 0; i < 15; ++i) {
    if (generator.has_next()) {
      std::cout << generator.next() << std::endl;
    } else {
      break;
    }
  }

  return 0;
}