#include "calc.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <exception>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <ctre-unicode.hpp>
#include <fmt/ranges.h>

namespace calc {
namespace {
Binary *bin_cast(WordPtr &w) { return dynamic_cast<Binary *>(w.get()); }
inline auto str_cast(std::u8string_view str) noexcept {
  return std::string_view(reinterpret_cast<const char *>(str.data()),
                          str.size());
}
inline auto str_cast(std::string_view str) noexcept {
  return std::u8string_view(reinterpret_cast<const char8_t *>(str.data()),
                            str.size());
}
auto makeWordPtr(auto &&tok) {
  using T = std::remove_cvref_t<decltype(tok)>;
  return WordPtr(new T(std::move(tok)));
};
WordPtr makeCon(std::string_view tok) {
  if (tok == "pi" || tok == "π") {
    return makeWordPtr(Constant(Constant::Types::pi));
  } else if (tok == "e") {
    return makeWordPtr(Constant(Constant::Types::e));
  } else if (tok == "tau" || tok == "τ") {
    return makeWordPtr(Constant(Constant::Types::tau));
  } else if (tok == "i") {
    return makeWordPtr(Constant(Constant::Types::i));
  }
  throw std::runtime_error("Unexpected token");
}
WordPtr makeOp(std::string_view tok) {
  if (tok == "+") {
    return makeWordPtr(Binary(Binary::Ops::add));
  } else if (tok == "-") {
    return makeWordPtr(Binary(Binary::Ops::sub));
  } else if (tok == "/") {
    return makeWordPtr(Binary(Binary::Ops::div));
  } else if (tok == "*") {
    return makeWordPtr(Binary(Binary::Ops::mul));
  } else if (tok == "^") {
    return makeWordPtr(Binary(Binary::Ops::exp));
  } else if (tok == "=") {
    return makeWordPtr(Binary(Binary::Ops::assign));
  } else if (tok == "√") {
    return makeWordPtr(Unary(Unary::Ops::sqrt));
  }
  throw std::runtime_error("Unexpected token");
}
WordPtr makeVar(std::string_view tok) { return WordPtr(new Variable(tok)); }
WordPtr makeNum(std::string_view num) {
  uint64_t val;
  auto result = std::from_chars(num.data(), num.data() + num.size(), val);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("number parsing failed");
  }
  return WordPtr(new Number(val, 1));
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
WordPtr makeDec(std::string_view num, std::string_view den) {
  uint64_t n;
  auto result = std::from_chars(num.data(), num.data() + num.size(), n);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("number parsing failed");
  }
  uint64_t d;
  result = std::from_chars(num.data(), num.data() + num.size(), d);
  if (result.ec != std::errc{}) {
    throw std::runtime_error("number parsing failed");
  }
  auto dec = decimal_places(d);
  return WordPtr(new Number(ipow10(n, dec) + d, dec));
}
} // namespace

std::vector<WordPtr> tokenize(std::string_view input) {
  std::vector<WordPtr> result;
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
      result.push_back(makeVar(str_cast(constant)));
    }
  }
  return result;
}

WordPtr parse(std::vector<WordPtr> s) {
  if (s.size() < 3) {
    return nullptr;
  }
  auto associate = [&](Binary::Ops op) {
    for (size_t i = 0; i != s.size(); ++i) {
      if (auto b = bin_cast(s[i]); b && b->op == op) {
        b->rhs = std::move(s[i - 1]);
        b->lhs = std::move(s[i + 1]);
        s.erase(s.begin() + i + 1);
        s.erase(s.begin() + i - 1);
        --i;
      }
    }
  };

  std::ranges::reverse(s);
  associate(Binary::Ops::exp);
  std::ranges::reverse(s);

  for (auto &&chunk : s | std::ranges::views::chunk_by([](auto &&a, auto &&b) {
                        auto ab = bin_cast(a);
                        auto a_b = !ab || ab->op == Binary::Ops::exp;
                        auto bb = bin_cast(b);
                        auto b_b = !bb || bb->op == Binary::Ops::exp;
                        return a_b && b_b;
                      })) {
    Word *head = chunk.front().get();
    for (auto &&word : chunk | std::views::drop(1)) {
      head->child = std::move(word);
      head = head->child.get();
    }
  }
  auto end = std::ranges::remove_if(s, [](auto &&a) { return !a.get(); });
  s.erase(end.begin(), end.end());

  using enum Binary::Ops;
  std::ranges::reverse(s);
  for (auto op = div; std::to_underlying(op) != std::to_underlying(assign) - 1;
       op = Binary::Ops(std::to_underlying(op) - 1)) {
    associate(op);
  }

  if (s.size() != 1) {
    throw std::runtime_error("Parsing error: more than one remains");
  }
  return std::move(s.front());
}
} // namespace calc