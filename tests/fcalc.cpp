#include "fast_calc/fcalc.hpp"
#include <fmt/ranges.h>

int main() {
  std::string input = "v = 3 * 2 + 1 - aπb ^ 2 / i";
  // v = 3 * 2 + 1 - aπb ^ 2 / i
  // v = 3 * 2 + 1 - aπ ^ b 2 / i
  // v = * 3 2 + 1 - aπ / ^ b 2 i
  // v = + * 3 2 - 1 aπ / ^ b 2 i
  // = v + * 3 2 - 1 aπ / ^ b 2 i
  auto a = fcalc::tokenize(input);
  fcalc::parse(a);
  fmt::print("out: {}\n", fmt::join(a, " "));
}