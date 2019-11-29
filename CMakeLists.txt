cmake_minimum_required(VERSION 3.15.5)

###
# setting
#set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "c compiler")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "c++ compiler")
set(CMAKE_C_COMPILER "/usr/bin/gcc-8" CACHE STRING "c compiler")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-8" CACHE STRING "c++ compiler")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

###
# project
project(tgimbox_project)

include_directories(include)

add_subdirectory(tgimbox)

add_executable(box-script test/box-script.cc)
set_property(TARGET box-script PROPERTY CXX_STANDARD 20)
target_link_libraries(box-script tgimbox)

message(STATUS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
