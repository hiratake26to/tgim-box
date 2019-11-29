cmake_minimum_required(VERSION 3.15.5)

###
# setting
set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "c compiler")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "c++ compiler")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

###
# project
project(tgimbox)

include_directories(../include)

file(GLOB_RECURSE src_files *.cc)
#add_library(tgimbox SHARED ${src_files}) # dynamic link (lib*.so)
add_library(tgimbox STATIC ${src_files}) # static link (lib*.a)
set_property(TARGET tgimbox PROPERTY CXX_STANDARD 20)

target_include_directories(tgimbox
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  )

