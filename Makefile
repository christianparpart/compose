CXX = clang++-3.9
CXXFLAGS = -O0 -ggdb #-stdlib=libc++

all: lazy_example1

lazy_example1: lazy_example1.cc lazy.h
	$(CXX) $(CXXFLAGS) -o $@ $< -std=c++14

clean:
	rm -f lazy_example1

.PHONY: clean all
