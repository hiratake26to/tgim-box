#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-g3 -std=c++2a $(INCLUDE)
INCLUDE=-I./include
SOURCES=$(wildcard ./src/src/*.cc)
OBJS=src/Box-exp.o $(SOURCES:.cc=.o)

Box-exp: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

debug:
	echo $(OBJS)

clean:
	rm -fr $(OBJS) Box-exp
