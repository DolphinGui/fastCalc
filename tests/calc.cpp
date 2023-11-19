#include "test/calc.hpp"
#include <fmt/ranges.h>
#include <ranges>

int main() {
  std::string input = "v = 3 * 2 + 1 - aπb ^ 2 / pi";
  // v = 3 * 2 + 1 - aπb ^ 2 / pi
  // = v + * 3 2 - 1 / aπ ^ b 2 pi
  auto a = calc::tokenize(input);
  fmt::print("pre:  {}\n", fmt::join(a, " "));
  auto b = calc::parse(a);
  fmt::print("post: {}\n", fmt::join(a, " "));
}