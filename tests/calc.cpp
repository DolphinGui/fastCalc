#include "test/calc.hpp"
#include <fmt/ranges.h>

namespace {
void print_recurse(calc::Word *w) {
  if (!w)
    return;
  fmt::print("{} ", w->format());
  if (auto b = dynamic_cast<calc::Binary *>(w)) {
    print_recurse(b->lhs.get());
  }
  print_recurse(w->child.get());
}
} // namespace

int main() {
  std::string input = "v = 3 * 2 + 1 - aπb ^ 2 / i";

  auto a = calc::tokenize(input);
  fmt::print("pre:  {}\n", fmt::join(a, " "));
  auto b = calc::parse(std::move(a));
  fmt::print("out: ");
  print_recurse(b.get());
  fmt::print("\n");
}