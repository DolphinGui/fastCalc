#include "fast_calc/fcalc.hpp"
#include <fmt/ranges.h>

int main() {
  std::string input = "v = 3 * 2 + 1 - aπb ^ 2 / pi";
  // v = 3 * 2 + 1 - aπb ^ 2 / pi
  // = v + * 3 2 - 1 / ^ aπb 2 pi
  auto a = fcalc::tokenize(input);
  fcalc::parse(a);
  fmt::print("out: {}\n", fmt::join(a, " "));
  fmt::print("{} offset: {}\n", a[9], a[9].bin.second_arg);
}