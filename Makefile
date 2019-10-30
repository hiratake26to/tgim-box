#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-g3 -std=c++2a $(INCLUDE)
INCLUDE=-I./include
OBJS=src/Box-exp.o

Box-exp: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -fr $(OBJS) Box-exp
