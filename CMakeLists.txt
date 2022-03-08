cmake_minimum_required(VERSION 3.21)
project(CppCoroutines)

set(CMAKE_CXX_STANDARD 20)

# list all c files in current dir recursively.
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_executable(${name} ${file})
endforeach()