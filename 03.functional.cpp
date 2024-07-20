//
// Created by benny on 2022/1/31.
//
#define __cpp_lib_coroutine
#include <coroutine>
#include <exception>
#include <iostream>
#include <thread>
#include <functional>
#include <list>
#include <string>
#include "io.h"

template<typename T>
struct Generator {

  class ExhaustedException : std::exception {};

  struct promise_type {
    T value;
    bool is_ready = false;

    std::suspend_never initial_suspend() { return {}; };

    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(T value) {
      this->value = value;
      is_ready = true;
      return {};
    }

    void unhandled_exception() {

    }

    Generator get_return_object() {
      return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    void return_void() {}
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

  T next() {
    if (has_next()) {
      handle.promise().is_ready = false;
      return handle.promise().value;
    }
    throw ExhaustedException();
  }
  
//  template<typename U>
//  Generator<U> map(std::function<U(T)> f) {
//    auto up_stream = std::move(*this);
//    while (up_stream.has_next()) {
//      co_yield f(up_stream.next());
//    }
//  }

  template<typename F>
  Generator<std::invoke_result_t<F, T>> map(F f) {
    auto up_steam = std::move(*this);
    while (up_steam.has_next()) {
      co_yield f(up_steam.next());
    }
  }

//  template<typename U>
//  Generator<U> flat_map(std::function<Generator<U>(T)> f) {
//    auto up_steam = std::move(*this);
//    while (up_steam.has_next()) {
//      auto generator = f(up_steam.next());
//      while (generator.has_next()) {
//        co_yield generator.next();
//      }
//    }
//  }

  template<typename F>
  std::invoke_result_t<F, T> flat_map(F f) {
    auto up_steam = std::move(*this);
    while (up_steam.has_next()) {
      auto generator = f(up_steam.next());
      while (generator.has_next()) {
        co_yield generator.next();
      }
    }
  }

  Generator take(int n) {
    auto up_steam = std::move(*this);
    int i = 0;
    while (i++ < n && up_steam.has_next()) {
      co_yield up_steam.next();
    }
  }

  template<typename F>
  Generator take_while(F f) {
    auto up_steam = std::move(*this);
    while (up_steam.has_next()) {
      T value = up_steam.next();
      if (f(value)) {
        co_yield value;
      } else {
        break;
      }
    }
  }

  template<typename F>
  Generator filter(F f) {
    auto up_steam = std::move(*this);
    while (up_steam.has_next()) {
      T value = up_steam.next();
      if (f(value)) {
        co_yield value;
      }
    }
  }

  template<typename F>
  void for_each(F f) {
    while (has_next()) {
      f(next());
    }
  }

  template<typename R, typename F>
  R fold(R initial, F f) {
    while (has_next()) {
      initial = f(initial, next());
    }
    return initial;
  }

  T sum() {
    T sum = 0;
    while (has_next()) {
      sum += next();
    }
    return sum;
  }

  Generator static from_array(T array[], int n) {
    for (int i = 0; i < n; ++i) {
      co_yield array[i];
    }
  }

  Generator static from_list(std::list<T> list) {
    for (auto t: list) {
      co_yield t;
    }
  }

  Generator static from(std::initializer_list<T> args) {
    for (auto t: args) {
      co_yield t;
    }
  }

  template<typename ...TArgs>
  Generator static from(TArgs ...args) {
    (co_yield args, ...);
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

Generator<int> fibonacci() {
  co_yield 0;
  co_yield 1;

  int a = 0;
  int b = 1;
  while (true) {
    co_yield a + b;
    b = a + b;
    a = b - a;
  }
}

int main() {
  Generator<int>::from(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
      .filter([](auto i) {
        std::cout << "filter: " << i << std::endl;
        return i % 2 == 0;
      })
      .map([](auto i) {
        std::cout << "map: " << i << std::endl;
        return i * 3;
      })
      .flat_map([](auto i) -> Generator<int> {
        std::cout << "flat_map: " << i << std::endl;
        for (int j = 0; j < i; ++j) {
          co_yield j;
        }
      }).take(3)
      .for_each([](auto i) {
        std::cout << "for_each: " << i << std::endl;
      });

  int array[] = {1, 2, 3, 4};
  Generator<int>::from_array(array, 4);
  Generator<int>::from_list(std::list{1, 2, 3, 4});
  Generator<int>::from({1, 2, 3, 4});

  Generator<int>::from(1, 2, 3, 4)
      .flat_map([](auto i) -> Generator<int> {
        for (int j = 0; j < i; ++j) {
          co_yield j;
        }
      })
      .for_each([](auto i) {
        if (i == 0) {
          std::cout << std::endl;
        }
        std::cout << "* ";
      });

  fibonacci().take_while([](auto i) {
    return i < 100;
  }).for_each([](auto i) {
    std::cout << i << " ";
  });

  std::cout << std::endl;

  std::cout << Generator<double>::from(1.0, 2.0, 3.0, 4, 5, 6.0f).sum() << std::endl;
  std::cout << Generator<double>::from(1.0, 2.0, 3.0, 4, 5, 6.0f).fold(1, [](auto acc, auto i) { return acc * i; })
            << std::endl;

  auto seq = fibonacci().map([](auto i) {
    return std::to_string(i);
  });

  for(int i = 0; i < 10; i++) {
    std::cout << seq.next() << std::endl;
  }

  Generator<int> generator = Generator<int>::from(1, 2, 3, 4).map([](int i) {
    return i * 2;
  });

  generator.for_each([](auto i) {
    std::cout << i << std::endl;
  });
  return 0;
}