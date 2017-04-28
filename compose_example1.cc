#include "compose.h"
#include <iostream>

int main() {
  const auto v = {1, 2, 3, 4, 5, 6, 7, 8};

  const auto range = compose(v)
                         .map([](auto x) { return x * x; })
                         .select([](auto x) { return !(x % 2); })
                         .take(3);

  std::cout << "count: " << range.size() << std::endl;

  range.each_with_index([](auto index, auto a) {
    std::cout << index << ": " << a << std::endl;
  });

  const auto sum = range.fold(0, [](auto a, auto b) { return a + b; });

  std::cout << "sum: " << sum << std::endl;

  const auto glue = range.fold(std::string(), [](auto a, auto b) {
    return a.empty() ? std::to_string(b)
                     : a + " " + std::to_string(b);
  });

  std::cout << "glue: " << glue << std::endl;
}
