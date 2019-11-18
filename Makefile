#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-g3 -std=c++2a $(INCLUDE)
INCLUDE=-I./include
SOURCES=$(wildcard ./src/src/*.cc)
PROGRAM=box-script
OBJS=src/$(PROGRAM).o $(SOURCES:.cc=.o)

$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

debug:
	echo $(OBJS)

clean:
	rm -fr $(OBJS) $(PROGRAM)
