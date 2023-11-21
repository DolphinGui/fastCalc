#include <ctre-unicode.hpp>
#include <fmt/core.h>
#include <string_view>
#include <random>

constexpr auto tokenize =
    ctre::range<R"((\d+)(?:\.(\d+))?|([+\-*/^()=√])|(pi|tau|[ieπτ])|(\S))">;
constexpr auto str =
    std::u8string_view(u8"v = 3 * 2 + 1 - aπb ^ 2 / i + 412.312");
inline auto str_cast(std::u8string_view str) noexcept {
  return std::string_view(reinterpret_cast<const char *>(str.data()),
                          str.size());
}
int main() {
  for (auto &&match : tokenize(str)) {
    if (auto num = match.get<1>()) {
      auto den = match.get<2>();
      if (!den) {
        fmt::print("number: {}\n", str_cast(num));
      } else {
        fmt::print("decimal: {}.{}\n", str_cast(num), str_cast(den));
      }
    } else if (auto op = match.get<3>()) {
      fmt::print("op: {}\n", str_cast(op));
    } else if (auto constant = match.get<4>()) {
      fmt::print("constant: {}\n", str_cast(constant));
    } else if (auto var = match.get<5>()) {
      fmt::print("var: {}\n", str_cast(var));
    }
  }
}