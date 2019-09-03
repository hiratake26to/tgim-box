#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-std=c++1z
OBJS=src/Box-exp.o

Box-exp: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -fr Box-exp.o Box-exp
