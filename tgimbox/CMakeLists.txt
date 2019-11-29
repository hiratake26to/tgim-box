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
project(tgimbox)

include_directories(../include)

file(GLOB_RECURSE src_files *.cc)
#add_library(tgimbox SHARED ${src_files}) # dynamic link (lib*.so)
#add_library(tgimbox STATIC ${src_files}) # static link (lib*.a)
add_library(tgimbox SHARED ${src_files}) # for pybind11
#set_property(TARGET tgimbox PROPERTY CXX_STANDARD 20) 
set_property(TARGET tgimbox PROPERTY CXX_STANDARD 17) # use for pybind11

target_include_directories(tgimbox
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  )

