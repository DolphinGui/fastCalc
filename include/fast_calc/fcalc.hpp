#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace fcalc {
#define X(Type, _) Type,
enum struct WordType : uint8_t {
#include "wordTypeX"
};
#undef X
inline const char *format_as(WordType t) noexcept {
  using enum WordType;
#define X(Type, _)                                                             \
  case Type:                                                                   \
    return #Type;
  switch (t) {
#include "wordTypeX"
  }
#undef X
}

inline bool is_value(WordType w) noexcept {
  using enum WordType;
  return w == Number || w == Constant || w == Variable;
}
struct Token {
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
};
struct Number {
  uint64_t num;
  int64_t den;
  bool operator==(const Number &t) const noexcept {
    return num == t.num && den == t.den;
  }
};
struct Constant {
  enum struct Types { pi, e, tau, i } type;
  Constant() = default;
  Constant(Types t) : type(t) {}
  bool operator==(const Constant &t) const noexcept { return type == t.type; }
};
struct Variable {
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
};
struct Unary {
  enum struct Ops { minus, sqrt } op;
  Unary() = default;
  Unary(Ops t) : op(t) {}
  bool operator==(const Unary &t) const noexcept { return op == t.op; }
};
struct Binary {
  enum Ops : uint8_t {
    assign,
    add,
    sub,
    mul,
    div,
    exp,
  } op;

  friend inline std::string_view format_as(Ops o) noexcept {
    switch (o) {
    case assign:
      return "assign";
    case add:
      return "add";
    case sub:
      return "sub";
    case mul:
      return "mul";
    case div:
      return "div";
    case exp:
      return "exp";
    default:
      return "unknown";
    }
  }

  // this is an offset that points to the second argument
  // ie + 2e 1, second_arg = 3
  uint8_t second_arg{};
  Binary() = default;
  Binary(Ops t) : op(t) {}
  bool operator==(const Binary &t) const noexcept { return op == t.op; }
};

struct Word {
  Word() : num{0, 0}, type(WordType::Number) {}

#define X(Type, member)                                                        \
  Word(Type &&t) : member(std::move(t)), type(WordType::Type) {}
#include "wordTypeX"
#undef X
#define X(Type, member)                                                        \
  Word(const Type &t) : member(t), type(WordType::Type) {}
#include "wordTypeX"
#undef X

  Word(std::string_view word) : tok(word), type{WordType::Token} {}
  // this can be done by macros

#define X(Type, member)                                                        \
  case Type:                                                                   \
    new (&member) struct Type(w.member);                                       \
    break;
  Word(const Word &w) : type(w.type) {
    switch (type) {
      using enum WordType;
#include "wordTypeX"
    }
  }
#undef X
#define X(Type, member)                                                        \
  case Type:                                                                   \
    new (&member) struct Type(std::move(w.member));                            \
    break;
  Word(Word &&w) : type(std::move(w.type)) {
    switch (type) {
      using enum WordType;
#include "wordTypeX"
    }
  }
#undef X
#define X(Type, member)                                                        \
  case Type:                                                                   \
    return member == other.member;
  bool operator==(const Word &other) const {
    if (type != other.type)
      return false;
    switch (type) {
      using enum WordType;
#include "wordTypeX"
    }
    throw std::runtime_error("Unhandled case in Word::operator==");
  }
#undef X

  // this delegates to copy constructor. this is also utterly sketchy.
  Word &operator=(const Word &other) {
    this->~Word();
    new (this) Word(other);
    return *this;
  }

  friend void swap(Word &a, Word &b) {
    auto tmp = Word(a);
    a = b;
    b = tmp;
  }

  Word &operator=(Word &&other) {
    swap(*this, other);
    return *this;
  }

#define X(Type, member)                                                        \
  case Type:                                                                   \
    member.~Type();                                                            \
    break;
  ~Word() noexcept {
    switch (type) {
      using enum WordType;
#include "wordTypeX"
    }
  }
#undef X

  union {
    Token tok;
    Number num;
    Constant con;
    Variable var;
    Unary un;
    Binary bin;
  };

  WordType type;
};
std::vector<Word> tokenize(std::string_view);
void parse(std::span<Word> s);
void resolve(std::span<Word> s);
} // namespace fcalc

#ifdef FCALC_FMT_FORMAT
template <>
struct fmt::formatter<fcalc::Token> : fmt::formatter<std::string_view> {
  constexpr auto format(const fcalc::Token &t, format_context &ctx) const
      -> format_context::iterator {
    return formatter<string_view>::format(t.s, ctx);
  }
};

template <> struct fmt::formatter<fcalc::Number> : formatter<double> {
  constexpr auto format(const fcalc::Number &n, format_context &ctx) const
      -> format_context::iterator {
    if (n.den % 10 == 0 || n.den == 1)
      return formatter<double>::format(double(n.num) / double(n.den), ctx);
    else
      return fmt::format_to(ctx.out(), "{}/{}", n.num, n.den);
  }
};

template <>
struct fmt::formatter<fcalc::Constant> : formatter<std::string_view> {
  constexpr auto format(const fcalc::Constant &c, format_context &ctx) const
      -> format_context::iterator {
    std::string_view result = "unknown";
    switch (c.type) {
      using enum fcalc::Constant::Types;
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
    return formatter<std::string_view>::format(result, ctx);
  }
};

template <>
struct fmt::formatter<fcalc::Variable> : fmt::formatter<std::string> {
  constexpr auto format(const fcalc::Variable &v, format_context &ctx) const
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "({})", v.s);
  }
};

template <> struct fmt::formatter<fcalc::Unary> : formatter<std::string_view> {
  constexpr auto format(const fcalc::Unary &u, format_context &ctx) const
      -> format_context::iterator {
    std::string_view result = "unknown";
    switch (u.op) {
    case fcalc::Unary::Ops::minus:
      result = "-";
      break;
    case fcalc::Unary::Ops::sqrt:
      result = "√";
      break;
    }
    return formatter<std::string_view>::format(result, ctx);
  }
};

template <> struct fmt::formatter<fcalc::Binary> : formatter<std::string_view> {
  constexpr auto format(const fcalc::Binary &b, format_context &ctx) const
      -> format_context::iterator {
    std::string_view result = "unknown";
    switch (b.op) {
      using enum fcalc::Binary::Ops;
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
    return formatter<std::string_view>::format(result, ctx);
  }
};

template <> struct fmt::formatter<fcalc::Word> : formatter<std::string_view> {
  constexpr auto format(const fcalc::Word &w, format_context &ctx) const
      -> format_context::iterator {
    switch (w.type) {
      using enum fcalc::WordType;
    case Token:
      return fmt::format_to(ctx.out(), "{}", w.tok);
    case Number:
      return fmt::format_to(ctx.out(), "{}", w.num);
    case Constant:
      return fmt::format_to(ctx.out(), "{}", w.con);
    case Variable:
      return fmt::format_to(ctx.out(), "{}", w.var);
    case Unary:
      return fmt::format_to(ctx.out(), "{}", w.un);
    case Binary:
      return fmt::format_to(ctx.out(), "{}", w.bin);
    }
    return fmt::format_to(ctx.out(), "?");
  }
};
#endif