#CXXFLAGS=-std=c++11
CXX=clang++
# use c++17
CXXFLAGS=-std=c++1z

Box-exp:

.PHONY: clean

clean:
	rm -fr Box-exp.o Box-exp
