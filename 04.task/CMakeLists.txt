cmake_minimum_required(VERSION 3.21)
project(CppCoroutines-Tasks)

set(CMAKE_CXX_STANDARD 20)

add_executable("coroutine-task"
        main.cpp
        io_utils.cpp)