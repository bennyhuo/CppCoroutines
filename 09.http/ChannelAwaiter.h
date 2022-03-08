//
// Created by benny on 2022/3/21.
//

#ifndef CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
#define CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_

#include "coroutine_common.h"

template<typename ValueType>
struct Channel;

template<typename ValueType>
struct WriterAwaiter {
  Channel<ValueType> *channel;
  AbstractExecutor *executor = nullptr;
  ValueType _value;
  std::coroutine_handle<> handle;

  bool await_ready() {
    return false;
  }

  auto await_suspend(std::coroutine_handle<> coroutine_handle) {
    this->handle = coroutine_handle;
    channel->try_push_writer(this);
  }

  void await_resume() {
    channel->check_closed();
  }

  void resume() {
    if (executor) {
      executor->execute([this]() { handle.resume(); });
    } else {
      handle.resume();
    }
  }
};

template<typename ValueType>
struct ReaderAwaiter {
  Channel<ValueType> *channel;
  AbstractExecutor *executor = nullptr;
  ValueType _value;
  ValueType* p_value = nullptr;
  std::coroutine_handle<> handle;

  bool await_ready() { return false; }

  auto await_suspend(std::coroutine_handle<> coroutine_handle) {
    this->handle = coroutine_handle;
    channel->try_push_reader(this);
  }

  int await_resume() {
    channel->check_closed();
    return _value;
  }

  void resume(ValueType value) {
    this->_value = value;
    if (p_value) {
      *p_value = value;
    }
    resume();
  }

  void resume() {
    if (executor) {
      executor->execute([this]() { handle.resume(); });
    } else {
      handle.resume();
    }
  }
};

#endif //CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
