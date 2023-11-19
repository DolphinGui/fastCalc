#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

namespace calc {
struct Word;
using WordPtr = std::unique_ptr<Word>;
struct Word {
  Word() = default;
  Word(WordPtr &&child) : child(std::move(child)) {}
  virtual std::string format() const = 0;
  virtual ~Word() = default;
  WordPtr child;
};

WordPtr WordFactory(std::string_view s);

struct Token : Word {
  std::string s;
  Token(std::string &&s) : s(std::move(s)) {}
  Token(std::string_view s) : s(std::move(s)) {}
  Token(Token &&other) : s(std::move(other.s)) {}
  Token &operator=(Token &&t) {
    std::swap(s, t.s);
    return *this;
  }
  Token(const Token &other) : s(other.s) {}
  Token &operator=(const Token &t) {
    s = t.s;
    return *this;
  }
  bool operator==(const Token &t) const noexcept { return s == t.s; }
  std::string format() const override final { return s; }
};

struct Number : Word {
  Number(int32_t num) noexcept : num(num), den(1) {
    if (num < 0)
      den = -1;
  }
  Number(uint64_t num, int64_t den) noexcept : num(num), den(den) {}
  Number() = default;
  Number(Number &&n) : Word(std::move(n.child)), num(n.num), den(n.den) {}
  uint64_t num;
  int64_t den;
  bool operator==(const Number &t) const noexcept {
    return num == t.num && den == t.den;
  }
  std::string format() const override final {
    if (den % 10 == 0 || den == 1)
      return fmt::format("{}", double(num) / double(den));
    else
      return fmt::format("{}", double(num) / double(den));
  }
};

struct Constant : Word {
  enum struct Types { pi, e, tau, i } type;
  Constant() = default;
  Constant(WordPtr child) : Word(std::move(child)) {}
  Constant(Constant &&n) : Word(std::move(n.child)), type(n.type) {}
  Constant(Types t) : type(t) {}
  bool operator==(const Constant &t) const noexcept { return type == t.type; }
  std::string format() const override final {
    std::string result = "unknown constant";
    switch (type) {
      using enum Types;
    case pi:
      result = "π";
      break;
    case e:
      result = "e";
      break;
    case tau:
      result = "τ";
      break;
    case i:
      result = "i";
      break;
    }
    return result;
  }
};

struct Variable : Word {
  std::string s;
  Variable(std::string &&s) : s(std::move(s)) {}
  Variable(std::string_view s) : s(std::move(s)) {}
  Variable(Token &&other) : s(std::move(other.s)) {}
  Variable &operator=(Variable &&t) {
    std::swap(s, t.s);
    return *this;
  }
  Variable(const Variable &other) : s(other.s) {}
  Variable &operator=(const Variable &t) {
    s = t.s;
    return *this;
  }
  bool operator==(const Variable &t) const noexcept { return s == t.s; }
  std::string format() const override final { return fmt::format("({})", s); }
};

struct Unary : Word {
  enum struct Ops { minus, sqrt } op;
  Unary() = default;
  Unary(Ops t) : op(t) {}
  Unary(Unary &&n) : Word(std::move(n.child)), op(n.op) {}
  bool operator==(const Unary &t) const noexcept { return op == t.op; }
  std::string format() const override final {
    std::string result = "unknown constant";
    switch (op) {
    case Ops::minus:
      result = "-";
      break;
    case Ops::sqrt:
      result = "√";
      break;
    }
    return result;
  }
};

struct Binary : Word {
  enum struct Ops : int8_t {
    assign,
    add,
    sub,
    mul,
    div,
    exp,
  } op;

  Binary() = default;
  Binary(Ops t) : op(t) {}
  Binary(Binary &&n) : Word(std::move(n.child)), op(n.op) {}
  bool operator==(const Binary &t) const noexcept { return op == t.op; }
  WordPtr lhs;
  std::string format() const override final {
    std::string result = "unknown constant";
    switch (op) {
      using enum Ops;
    case add:
      result = "+";
      break;
    case sub:
      result = "-";
      break;
    case mul:
      result = "*";
      break;
    case div:
      result = "/";
      break;
    case exp:
      result = "^";
      break;
    case assign:
      result = "=";
      break;
    }
    return result;
  }
};

std::vector<WordPtr> tokenize(std::string_view);
WordPtr parse(std::vector<WordPtr> s);
WordPtr resolve(WordPtr s);
} // namespace calc

#ifdef FCALC_FMT_FORMAT
template <typename T>
struct fmt::formatter<
    T, std::enable_if_t<std::is_base_of<calc::Word, T>::value, char>>
    : fmt::formatter<std::string> {
  auto format(const calc::Word &a, format_context &ctx) const {
    return fmt::formatter<std::string>::format(a.format(), ctx);
  }
};

template <> struct fmt::formatter<calc::Word *> : fmt::formatter<std::string> {
  auto format(calc::Word const *a, format_context &ctx) const {
    if (a)
      return fmt::formatter<std::string>::format(a->format(), ctx);
    else
      return fmt::formatter<std::string>::format("", ctx);
  }
};
template <> struct fmt::formatter<calc::WordPtr> : fmt::formatter<std::string> {
  auto format(calc::WordPtr const &a, format_context &ctx) const {
    if (a)
      return fmt::formatter<std::string>::format(a->format(), ctx);
    else
      return fmt::formatter<std::string>::format("", ctx);
  }
};

template <>
struct fmt::formatter<calc::Binary::Ops> : fmt::formatter<std::string_view> {
  constexpr auto format(calc::Binary::Ops a, format_context &ctx) const {
    std::string_view val = "?";
    switch (a) {
      using enum calc::Binary::Ops;
    case assign:
      val = "assign";
      break;
    case add:
      val = "add";
      break;
    case sub:
      val = "sub";
      break;
    case mul:
      val = "mul";
      break;
    case div:
      val = "div";
      break;
    case exp:
      val = "exp";
      break;
    }
    return fmt::formatter<std::string_view>::format(val, ctx);
  }
};

#endif