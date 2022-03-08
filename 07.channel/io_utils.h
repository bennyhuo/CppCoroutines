//
// Created by benny on 2022/3/10.
//

#ifndef CPPCOROUTINES__IO_H_
#define CPPCOROUTINES__IO_H_

#include <iostream>
#include <sstream>

const char *file_name(const char *path);

std::stringstream &PrintTime(std::stringstream &ss);

std::stringstream &PrintThread(std::stringstream &ss);

template<typename ... U>
void Println(std::stringstream &ss, U... u) {
  int i = 0;
  auto printer = [&ss, &i]<typename Arg>(Arg arg) {
    if (sizeof...(U) == ++i) ss << arg << std::endl;
    else ss << arg << " ";
  };
  (printer(u), ...);

  std::cout << ss.str();
  std::cout.flush();
}

#define debug(...) \
std::stringstream ss;\
PrintTime(ss);       \
PrintThread(ss);   \
char buf[100];                   \
size_t len = snprintf(buf, 100, "(%s:%d) %s: ", file_name(__FILE__), __LINE__, __func__); \
std::string s(buf, buf + len - 1);                                                       \
Println(ss, s, __VA_ARGS__);

#endif //CPPCOROUTINES__IO_H_
