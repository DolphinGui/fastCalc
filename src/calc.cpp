#include "calc.hpp"
#include "re.hpp"

#include "fmt/format.h"
#include "fmt/ranges.h"
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace calc {
std::vector<WordPtr> tokenize(std::string_view input) {
  std::vector<WordPtr> result;
  auto p = re::Pattern(R"((?:\d+)(?:.\d+)?|[+\-*/^()ie=]|pi|tau|\S)",
                       re::Options::ucp | re::Options::utf);
  result.reserve(20);
  for (auto match : p.match(input)) {
    result.push_back(WordFactory(match));
  }
  return result;
}

namespace {

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
  return Result{true, std::move(num)};
}

Binary *bin_cast(WordPtr &w) { return dynamic_cast<Binary *>(w.get()); };

} // namespace

WordPtr WordFactory(std::string_view tok) {
  auto makeWordPtr = [](auto &&tok) {
    using T = std::remove_cvref_t<decltype(tok)>;
    return WordPtr(new T(std::move(tok)));
  };
  auto result = parse_number(tok);
  if (result.is_number) {
    return makeWordPtr(std::move(result.n));
  }

  if (tok == "pi" || tok == "π") {
    return makeWordPtr(Constant(Constant::Types::pi));
  } else if (tok == "e") {
    return makeWordPtr(Constant(Constant::Types::e));
  } else if (tok == "tau" || tok == "τ") {
    return makeWordPtr(Constant(Constant::Types::tau));
  } else if (tok == "i") {
    return makeWordPtr(Constant(Constant::Types::i));
  } else if (tok == "+") {
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
  return makeWordPtr(Variable(tok));
}

WordPtr parse(std::vector<WordPtr> s) {
  if (s.size() >= 3) {

    // exp has higher precedence than implicit mult, so it must be done first
    for (size_t i = 0; i != s.size(); ++i) {
      if (auto b = bin_cast(s[i]); b && b->op == Binary::Ops::exp) {
        b->lhs = std::move(s[i - 1]);
        b->child = std::move(s[i + 1]);
        s.erase(s.begin() + i + 1);
        s.erase(s.begin() + i - 1);
        --i;
      }
    }

    for (auto &&chunk :
         s | std::ranges::views::chunk_by([](auto &&a, auto &&b) {
           auto ab = bin_cast(a);
           auto bb = bin_cast(b);
           if (!ab && !bb)
             return true;
           if (!ab && bb->op == Binary::Ops::exp)
             return true;
           return false;
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
    for (auto op = div;
         std::to_underlying(op) != std::to_underlying(assign) - 1;
         op = Binary::Ops(std::to_underlying(op) - 1)) {
      for (size_t i = 0; i != s.size(); ++i) {
        if (auto b = bin_cast(s[i]); b && b->op == op) {
          b->lhs = std::move(s[i - 1]);
          b->child = std::move(s[i + 1]);
          s.erase(s.begin() + i + 1);
          s.erase(s.begin() + i - 1);
          --i;
        }
      }
    }
    if (s.size() != 1) {
      throw std::runtime_error("Parsing error: more than one remains");
    }
    return std::move(s.front());
  }
  return nullptr;
}
} // namespace calc