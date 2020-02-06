cmake_minimum_required(VERSION 3.0)

#set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "c compiler")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "c++ compiler")
set(CMAKE_C_COMPILER "/usr/bin/gcc-8" CACHE STRING "c compiler")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-8" CACHE STRING "c++ compiler")
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")

project(tgimbox-pybind)

## bind
set(PYBIND11_CPP_STANDARD -std=c++17)
#set(PYBIND11_CPP_STANDARD -std=c++2a)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tgimbox ${CMAKE_CURRENT_BINARY_DIR}/tgimbox)
add_subdirectory(pybind11)
#add_subdirectory(pybind11_json-0.2.2)

## lib
include_directories(../include ./pybind11_json/include)
file(GLOB_RECURSE src_files ../src/src/*.cc)
#add_library(tgimboxcore MODULE ${src_files} tgimbox-bind.cc)
add_library(tgimboxcore MODULE ${src_files} tgimbox-bind.cc)
#set_property(TARGET tgimbox PROPERTY CXX_STANDARD 20)

target_link_libraries(tgimboxcore
  PUBLIC tgimbox
  PRIVATE pybind11::module
  )
set_target_properties(tgimboxcore 
  PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}"
  SUFFIX "${PYTHON_MODULE_EXTENSION}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  )

configure_file(tgimbox.py ${CMAKE_BINARY_DIR}/lib/tgimbox.py COPYONLY)

#pybind11_add_module(tgimbox MODULE tgimbox-bind.cc)
#set_property(TARGET tgimbox-lib PROPERTY CXX_STANDARD 17)

