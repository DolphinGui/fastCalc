#include "fcalc.hpp"
#include "re.hpp"

#include <charconv>
#include <fmt/ranges.h>
#include <pcre2.h>
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace fcalc {
std::vector<Word> tokenize(std::string_view input) {
  std::vector<Word> result;
  auto p = re::Pattern(R"((\d+)|[+\-*/^()ie=]|pi|tau|\S)",
                       re::Options::ucp | re::Options::utf);
  result.reserve(20);
  for (auto match : p.match(input)) {
    result.push_back(Word(match));
  }
  return result;
}
namespace {
inline bool operator==(const Token &lhs, std::string_view rhs) {
  return lhs.s == rhs;
}
Word parse_token(const Word &w) {
  auto &tok = w.tok;
  double val;
  // I am too lazy to write an arbitray number parser
  auto result = std::from_chars(tok.s.data(), tok.s.data() + tok.s.size(), val);
  if (result.ec != std::errc::invalid_argument) {
    if (result.ec == std::errc::result_out_of_range) {
      throw std::runtime_error("Parsed number is out of range");
    }
    double integral{};
    if (std::modf(val, &integral) == 0.0) {
      return Number{int64_t(integral), 1};
    }
  }
  if (tok == "pi" || tok == "π") {
    return Word(Constant::Types::pi);
  } else if (tok == "e") {
    return Word(Constant::Types::e);
  } else if (tok == "tau" || tok == "τ") {
    return Word(Constant::Types::tau);
  } else if (tok == "i") {
    return Word(Constant::Types::i);
  } else if (tok == "+") {
    return Word(Binary::Ops::add);
  } else if (tok == "-") {
    return Word(Binary::Ops::sub);
  } else if (tok == "/") {
    return Word(Binary::Ops::div);
  } else if (tok == "*") {
    return Word(Binary::Ops::mult);
  } else if (tok == "^") {
    return Word(Binary::Ops::exp);
  } else if (tok == "=") {
    return Word(Binary::Ops::assign);
  } else if (tok == "√") {
    return Word(Unary::Ops::sqrt);
  }
  return Word(Variable{tok.s});
}
} // namespace
void parse(std::span<Word> s) {
  for (auto &w : s) {
    w = parse_token(w);
  }
  // (2 + e + g) / -pi
  // / + 2 + e g pi
  for (auto &&a : s | std::ranges::views::slide(2)) {
    if (a[1] == Word(Binary::Ops::sub) && !is_value(a[0].type)) {
      a[1] = Word(Unary::Ops::minus);
    }
  }
}
void resolve(std::span<Word> s);
} // namespace fcalc