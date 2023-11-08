#include "fast_calc/fcalc.hpp"
#include <fmt/ranges.h>

int main(){
  std::string input ="Σab + 2 / pi";
  auto a = fcalc::tokenize(input);
  fmt::print("out: {}\n", fmt::join(a, " "));
}