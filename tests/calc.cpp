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
  constexpr const char *input =
      "41^504/"
      "52+tautautau-3313-tautautauipi-41-tautautaupii^"
      "41/287^i+41^41/53/"
      "54*pipipi+itaupitautau^tauipi*64/i+64+14+itaupi/"
      "tau*75*ipitaupipi-tautautauipi+86/"
      "643^ipitauii-74/"
      "ipii*iitautaui^63";

  auto a = calc::tokenize(input);


  fmt::print("a:  {}\n", fmt::join(a, " "));
  auto root = calc::parse(std::move(a));
  fmt::print("out: ");
  print_recurse(root.get());
  fmt::print("\n");
}