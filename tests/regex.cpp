#include <ctre-unicode.hpp>
#include <fmt/core.h>
#include <string_view>

// namespace {
// const char *ops[] = {"-", "+", "*", "/", "^"};
// const char *rand_op(auto &r) {
//   std::uniform_int_distribution<uint32_t> rand(0, 4);
//   return ops[rand(r)];
// }
// const char *constants[] = {"pi", "tau", "i"};
// const char *rand_con(auto &r) {
//   std::uniform_int_distribution<uint32_t> rand(0, 2);
//   return constants[rand(r)];
// }
// void rand_term(std::string &expression, auto &r) {
//   std::uniform_int_distribution<uint32_t> rand(1, 6);
//   uint32_t term_length = rand(r);
//   std::uniform_int_distribution<uint32_t> rbool(0, 1);
//   for (uint32_t i = 0; i != term_length; ++i) {
//     if (rbool(r)) {
//       expression.append(rand_con(r));
//     } else {
//       std::uniform_int_distribution<uint32_t> i(1, 50);
//       expression.append(std::to_string(i(r)));
//     }
//   }
// }
// } // namespace

// int main() {
//   std::random_device r;
//   std::default_random_engine e(r());
//   std::string expression;
//   expression.reserve(500);
//   for (int i = 0; i != 200; i++) {
//     rand_term(expression, e);
//     expression.append(rand_op(e));
//   }
//   rand_term(expression, e);
//   fmt::print("{}\n", expression);
// }
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