#include "compose.h"
#include <iostream>

int main() {
  auto v = {1, 2, 3, 4, 5, 6, 7, 8};

  auto range = compose(v)
                   .map([](auto x) { return x * x; })
                   .select([](auto x) { return !(x % 2); })
                   .take(3);

  std::cout << "count: " << range.size() << std::endl;

  range.each_with_index([](auto index, auto a) {
    std::cout << index << ": " << a << std::endl;
  });
}
