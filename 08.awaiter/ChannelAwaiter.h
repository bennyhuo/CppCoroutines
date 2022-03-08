//
// Created by benny on 2022/3/21.
//

#ifndef CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
#define CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_

#include "coroutine_common.h"
#include "CommonAwaiter.h"

template<typename ValueType>
struct Channel;

template<typename ValueType>
struct WriterAwaiter : public Awaiter<void> {
  Channel<ValueType> *channel;
  ValueType _value;

  WriterAwaiter(Channel<ValueType> *channel, ValueType value) : channel(channel), _value(value) {}

  WriterAwaiter(WriterAwaiter &&other) noexcept
      : Awaiter(other),
        channel(std::exchange(other.channel, nullptr)),
        _value(other._value) {}

  void after_suspend() override {
    channel->try_push_writer(this);
  }

  void before_resume() override {
    channel->check_closed();
    channel = nullptr;
  }

  ~WriterAwaiter() {
    if (channel) channel->remove_writer(this);
  }
};

template<typename ValueType>
struct ReaderAwaiter : public Awaiter<ValueType> {
  Channel<ValueType> *channel;
  ValueType *p_value = nullptr;

  explicit ReaderAwaiter(Channel<ValueType> *channel) : Awaiter<ValueType>(), channel(channel) {}

  ReaderAwaiter(ReaderAwaiter &&other) noexcept
      : Awaiter<ValueType>(other),
        channel(std::exchange(other.channel, nullptr)),
        p_value(std::exchange(other.p_value, nullptr)) {}

  void after_suspend() override {
    channel->try_push_reader(this);
  }

  void before_resume() override {
    channel->check_closed();
    if (p_value) {
      *p_value = this->_result->get_or_throw();
    }
    channel = nullptr;
  }

  ~ReaderAwaiter() {
    if (channel) channel->remove_reader(this);
  }
};

#endif //CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
