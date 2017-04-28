#include "lazy.h"
#include <iostream>

int main() {
  auto v = {1, 2, 3, 4, 5, 6, 7, 8};

  auto range = lazy(v).
    map([](auto x) { return x * x; }).
    map([](auto x) { return -x; }).
    select([](auto x) { return !(x % 2); }).
    take(3);

  std::cout << "count: " << range.size() << std::endl;

  for (auto a: range) {
    std::cout << "a: " << a << std::endl;
  }
}
