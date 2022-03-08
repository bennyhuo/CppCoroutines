cmake_minimum_required(VERSION 3.21)
project(CppCoroutines-Tasks)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(httpdemo
        main.cpp
        io_utils.cpp)

target_link_libraries(httpdemo ${CONAN_LIBS})
