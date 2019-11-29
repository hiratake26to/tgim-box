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
all: CMakeLists.txt build
	cd build && cmake .. && make

build:
	mkdir build

.PHONY: clean debug

debug:
	./build/box-script
# use make
#clean:
#	rm -fr $(OBJS) $(PROGRAM)
# use cmake
clean:
	rm -fr build
