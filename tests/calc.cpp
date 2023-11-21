#include "test/calc.hpp"
#include <fmt/ranges.h>

namespace {
std::string print_recurse(calc::Word *w) {
  std::string result;
  if (!w)
    return result;
  result = fmt::format("{} ", w->format());
  if (auto b = dynamic_cast<calc::Binary *>(w)) {
    result.append(print_recurse(b->lhs.get()));
    result.append(print_recurse(b->rhs.get()));
  }
  result.append(print_recurse(w->child.get()));
  return result;
}
} // namespace

int main() {
  constexpr const char *input = "v = 3 * 2 + 1 - aπb ^ 2 / i";
  // = v + * 3 2 - 1 aπ / ^ b 2 i

  auto a = calc::tokenize(input);
  auto root = calc::parse(std::move(a));
  fmt::print("out: {}\n", print_recurse(root.get()));
  fmt::print("child: {}\n", print_recurse(root->child.get()));
  fmt::print(
      "lhs: {}\n",
      print_recurse(dynamic_cast<calc::Binary *>(root.get())->lhs.get()));
  fmt::print(
      "rhs: {}\n",
      print_recurse(dynamic_cast<calc::Binary *>(root.get())->rhs.get()));
}