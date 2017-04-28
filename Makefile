CXX = clang++-3.9
CXXFLAGS = -O0 -ggdb #-stdlib=libc++

all: compose_example1

compose_example1: compose_example1.cc compose.h
	$(CXX) $(CXXFLAGS) -o $@ $< -std=c++14

clean:
	rm -f compose_example1

.PHONY: clean all
