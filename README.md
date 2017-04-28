### Lazy

That's my little library for allowing myself to code in fuctional style in C++14.

### Example
```!cpp
#include "lazy.h"
#include <iostream>

int main() {
  auto v = {1, 2, 3, 4, 5, 6, 7, 8};

  auto range = lazy(v).
    map([](auto x) { return x * x; }).
    map([](auto x) { return -x; }).
    select([](auto x) { return !(x % 2); }).
    take(3);

  for (auto a: range) {
    std::cout << "a: " << a << std::endl;
  }
}
```

### Functions API

- `map(mutator)`
- `select(predicate)`
- `take(count)`
- ... (more to come, by dragons)
