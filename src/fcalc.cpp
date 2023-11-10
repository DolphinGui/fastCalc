#include "fcalc.hpp"
#include "re.hpp"

#include <algorithm>
#include <fmt/ranges.h>
#include <iterator>
#include <pcre2.h>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

namespace fcalc {
std::vector<Word> tokenize(std::string_view input) {
  std::vector<Word> result;
  auto p = re::Pattern(R"((?:\d+)(?:.\d+)?|[+\-*/^()ie=]|pi|tau|\S)",
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

int interp_digit(char c) {
  switch (c) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case '.':
    return -2;
  default:
    return -1;
  }
}
struct Result {
  bool is_number;
  Number n;
};

Result parse_number(std::string_view sv) {
  Number num{0, 1};
  auto iterator = sv.begin();
  if (*iterator == '-') {
    num.den = -1;
    ++iterator;
  }
  {
    auto c = interp_digit(*iterator++);
    if (c == -1)
      return {false};
    num.num += c;
    while (iterator != sv.end()) {
      int c = interp_digit(*iterator++);
      if (c == -2)
        break;
      num.num += c;
      num.num *= 10;
    }
  }
  while (iterator != sv.end()) {
    int c = interp_digit(*iterator++);
    num.num += c;
    num.num *= 10;
    num.den *= 10;
  }
  return Result{true, num};
}

Word parse_token(const Word &w) {
  auto &tok = w.tok;
  auto result = parse_number(tok.s);
  if (result.is_number) {
    return Word(result.n);
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
    return Word(Binary::Ops::mul);
  } else if (tok == "^") {
    return Word(Binary::Ops::exp);
  } else if (tok == "=") {
    return Word(Binary::Ops::assign);
  } else if (tok == "√") {
    return Word(Unary::Ops::sqrt);
  }
  return Word(Variable{tok.s});
}

// the algorithm basically implements a binary-search-like pattern,
// dividing it up from weakest operand to strongest
inline void bin_prefix(std::span<Word> terms, std::span<Word>::iterator op,
                       Binary::Ops optype, std::span<Word> fulldebug) {
  auto left = std::span(terms.begin(), op);
  for (auto lop = optype; lop < Binary::Ops::exp + 1;
       lop = static_cast<Binary::Ops>(lop + 1)) {
    auto pivot = std::ranges::find_if(left, [&](auto &&t) {
      return t.type == WordType::Binary && t.bin.op == lop;
    });
    if (pivot != left.end()) {
      bin_prefix(left, pivot, lop, fulldebug);
      break;
    }
  }
  auto right = std::span(op + 1, terms.end());
  for (auto rop = optype; rop < Binary::Ops::exp + 1;
       rop = static_cast<Binary::Ops>(rop + 1)) {
    auto pivot = std::ranges::find_if(right, [&](auto &&t) {
      return t.type == WordType::Binary && t.bin.op == rop;
    });
    if (pivot != right.end()) {
      bin_prefix(right, pivot, rop, fulldebug);
      break;
    }
  }
  op->bin.second_arg = std::distance(terms.begin(), op) + 1;
  std::ranges::rotate(std::span(terms.begin(), op + 1), op);
}

} // namespace
void parse(std::span<Word> s) {
  for (auto &w : s) {
    w = parse_token(w);
  }
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
    auto smallest = std::ranges::min_element(s, [](auto &&a, auto &&b) {
      auto a_bin = a.type != WordType::Binary;
      auto b_bin = b.type != WordType::Binary;
      if (a_bin && !b_bin) {
        return false;
      } else if (!a_bin && b_bin)
        return true;
      else if (!a_bin && !b_bin)
        return false;
      return std::to_underlying(a.type) < std::to_underlying(b.type);
    });
    bin_prefix(s, smallest, Binary::Ops::assign, s);
  }
}
void resolve(std::span<Word> s);
} // namespace fcalc