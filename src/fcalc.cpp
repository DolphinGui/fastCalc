#include "fcalc.hpp"
#include "ctre-unicode.hpp"

#include <algorithm>
#include <charconv>
#include <iterator>
#include <pcre2.h>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

namespace fcalc {

namespace {
inline auto str_cast(std::u8string_view str) noexcept {
  return std::string_view(reinterpret_cast<const char *>(str.data()),
                          str.size());
}
inline auto str_cast(std::string_view str) noexcept {
  return std::u8string_view(reinterpret_cast<const char8_t *>(str.data()),
                            str.size());
}

Word makeCon(std::string_view tok) {
  if (tok == "pi" || tok == "π") {
    return Constant(Constant::Types::pi);
  } else if (tok == "e") {
    return Constant(Constant::Types::e);
  } else if (tok == "tau" || tok == "τ") {
    return Constant(Constant::Types::tau);
  } else if (tok == "i") {
    return Constant(Constant::Types::i);
  }
  throw std::runtime_error("Unexpected token");
}
Word makeOp(std::string_view tok) {
  if (tok == "+") {
    return Binary(Binary::Ops::add);
  } else if (tok == "-") {
    return Binary(Binary::Ops::sub);
  } else if (tok == "/") {
    return Binary(Binary::Ops::div);
  } else if (tok == "*") {
    return Binary(Binary::Ops::mul);
  } else if (tok == "^") {
    return Binary(Binary::Ops::exp);
  } else if (tok == "=") {
    return Binary(Binary::Ops::assign);
  } else if (tok == "√") {
    return Unary(Unary::Ops::sqrt);
  }
  throw std::runtime_error("Unexpected token");
}
Word makeNum(std::string_view num) {
  uint64_t val;
  auto result = std::from_chars(num.data(), num.data() + num.size(), val);
  if (result.ec != std::errc{}) {
    throw std::runtime_error(fmt::format("number parsing failed: {}", num));
  }
  return Number(val, 1);
}

int decimal_places(uint64_t i) {
  int dec = 0;
  while (i != 0) {
    i %= 10;
    ++dec;
  }
  return dec;
}
uint64_t ipow10(uint64_t b, uint64_t e) {
  while (e != 0) {
    b *= 10;
    --e;
  }
  return b;
}
Word makeDec(std::string_view num, std::string_view den) {
  uint64_t n;
  auto result = std::from_chars(num.data(), num.data() + num.size(), n);
  if (result.ec != std::errc{}) {
    throw std::runtime_error(
        fmt::format("decimal integer parsing failed: {} . {}", num, den));
  }
  uint64_t d;
  result = std::from_chars(num.data(), num.data() + num.size(), d);
  if (result.ec != std::errc{}) {
    throw std::runtime_error(
        fmt::format("decimal parsing failed: {} . {}", num, den));
  }
  auto dec = decimal_places(d);
  return Number(ipow10(n, dec) + d, dec);
}
} // namespace

std::vector<Word> tokenize(std::string_view input) {
  std::vector<Word> result;
  constexpr auto tokenize =
      ctre::range<R"((\d+)(?:\.(\d+))?|([+\-*/^()=√])|(pi|tau|[ieπτ])|(\S))">;
  result.reserve(20);
  for (auto &&match : tokenize(str_cast(input))) {
    if (auto num = match.get<1>()) {
      auto den = match.get<2>();
      if (!den) {
        result.push_back(makeNum(str_cast(num)));
      } else {
        result.push_back(makeDec(str_cast(num), str_cast(den)));
      }
    } else if (auto op = match.get<3>()) {
      result.push_back(makeOp(str_cast(op)));
    } else if (auto constant = match.get<4>()) {
      result.push_back(makeCon(str_cast(constant)));
    } else if (auto var = match.get<5>()) {
      result.push_back(Variable(str_cast(var)));
    }
  }
  return result;
}
// the algorithm basically implements a binary-search-like pattern,
// dividing it up from weakest operand to strongest
inline void bin_prefix(std::span<Word> terms, std::span<Word>::iterator op,
                       Binary::Ops optype) {
  auto left = std::span(terms.begin(), op);
  auto right = std::span(op + 1, terms.end());
  using std::to_underlying;

  auto recurse = [&](auto &range) {
    for (auto op_t = optype;
         to_underlying(op_t) < to_underlying(Binary::Ops::exp) + 1;
         op_t = static_cast<Binary::Ops>(to_underlying(op_t) + 1)) {
      auto pivot = std::ranges::find_if(range, [&](auto &&t) {
        return t.type == WordType::Binary && t.bin.op == op_t;
      });
      if (pivot != range.end()) {
        bin_prefix(range, pivot, op_t);
        break;
      }
    }
  };

  recurse(left);
  recurse(right);
  if (optype != Binary::Ops::exp) {
    op->bin.second_arg = std::distance(terms.begin(), op) + 1;
    std::ranges::rotate(std::span(terms.begin(), op + 1), op);
  } else {
    op->bin.second_arg = 2;
    std::swap(*op, op[-1]);
  }
}
auto find_smallest(std::span<Word> s) {
  std::span<Word>::iterator smallest = s.begin() + 1;
  for (auto it = s.begin() + 1; it != s.end(); ++it) {
    if (it->type == WordType::Binary) {
      if (smallest->type != WordType::Binary) {
        smallest = it;
      } else if (std::to_underlying(it->bin.op) <
                 std::to_underlying(smallest->bin.op))
        smallest = it;
    }
  }
  return smallest;
}
void parse(std::span<Word> s) {
  if (s.size() >= 2) {
    if (s.front() == Word(Binary::Ops::sub))
      s.front() = Word(Unary::Ops::minus);
    for (auto &&a : s | std::ranges::views::slide(2)) {
      if (a[1] == Word(Binary::Ops::sub) && !is_value(a[0].type)) {
        a[1] = Word(Unary::Ops::minus);
      }
    }
  }
  if (s.size() >= 3) {
    auto smallest = find_smallest(s);
    bin_prefix(s, smallest, smallest->bin.op);
  }
}
void resolve(std::span<Word> s);
} // namespace fcalc