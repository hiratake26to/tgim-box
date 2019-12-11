#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-g3 -std=c++2a $(INCLUDE)
INCLUDE=-I./include
SOURCES=$(wildcard ./src/src/*.cc)
PROGRAM=box-script
OBJS=box-script/$(PROGRAM).o $(SOURCES:.cc=.o)

# use normal `make` command
#$(PROGRAM): $(OBJS)
#	$(CXX) $(CXXFLAGS) -o $@ $^

# use cmake
all: ccapi pybind

# C++ API
ccapi: CMakeLists.txt build
	cd build && cmake .. && $(MAKE)
	@echo "[ccapi] Build for C++ API Success!"

build:
	mkdir build

# Python binding
pybind: FORCE
	cd $@ && $(MAKE)
	@echo "[pybind] Build for Python binding Success!"

FORCE:

.PHONY: clean debug

debug: all
	./build/box-script

lldb: all
	lldb ./build/box-script
# use make
#clean:
#	rm -fr $(OBJS) $(PROGRAM)
# use cmake
clean:
	rm -fr build
	cd pybind && make clean
