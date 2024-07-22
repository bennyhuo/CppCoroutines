//
// Created by benny on 2022/3/26.
//

#ifndef CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_
#define CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_

#include "Executor.h"
#include "Result.h"
#include "coroutine_common.h"

template<typename R>
struct Awaiter {

  using ResultType = R;

  bool await_ready() const { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    this->_handle = handle;
    after_suspend();
  }

  R await_resume() {
    before_resume();
    return _result->get_or_throw();
  }

  void resume(R value) {
    dispatch([this, value]() {
      _result = Result<R>(static_cast<R>(value));
      _handle.resume();
    });
  }

  void resume_unsafe() {
    dispatch([this]() { _handle.resume(); });
  }

  void resume_exception(std::exception_ptr &&e) {
    dispatch([this, e]() {
      _result = Result<R>(static_cast<std::exception_ptr>(e));
      _handle.resume();
    });
  }

  void install_executor(AbstractExecutor *executor) {
    _executor = executor;
  }

 protected:
  std::optional<Result<R>> _result{};

  virtual void after_suspend() {}

  virtual void before_resume() {}
 private:
  AbstractExecutor *_executor = nullptr;
  std::coroutine_handle<> _handle = nullptr;

  void dispatch(std::function<void()> &&f) {
    if (_executor) {
      _executor->execute(std::move(f));
    } else {
      f();
    }
  }
};

template<>
struct Awaiter<void> {

  using ResultType = void;

  bool await_ready() { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    this->_handle = handle;
    after_suspend();
  }

  void await_resume() {
    before_resume();
    _result->get_or_throw();
  }

  void resume() {
    dispatch([this]() {
      _result = Result<void>();
      _handle.resume();
    });
  }

  void resume_unsafe() {
    dispatch([this]() { _handle.resume(); });
  }

  void resume_exception(std::exception_ptr &&e) {
    dispatch([this, e]() {
      _result = Result<void>(static_cast<std::exception_ptr>(e));
      _handle.resume();
    });
  }

  void install_executor(AbstractExecutor *executor) {
    _executor = executor;
  }

  virtual void after_suspend() {}

  virtual void before_resume() {}

 private:
  std::optional<Result<void>> _result{};
  AbstractExecutor *_executor = nullptr;
  std::coroutine_handle<> _handle = nullptr;

  void dispatch(std::function<void()> &&f) {
    if (_executor) {
      _executor->execute(std::move(f));
    } else {
      f();
    }
  }
};

#endif //CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_
