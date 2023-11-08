#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <span>
#include <string>
#include <vector>

namespace fcalc {
enum struct WordType : uint8_t {
  Token,
  Number,
  Constant,
  Variable,
  Unary,
  Binary,
};
struct Token {
  const char *s;
  size_t size{};
};
struct Number {
  int64_t num, den;
};
struct Constant {
  enum struct Types { pi, e, tau, i } types;
};
struct Variable {
  const char *s;
  size_t size{};
};
struct Unary {
  enum struct Ops { minus, sqrt } op;
};
struct Binary {
  enum struct Ops { add, sub, mult, div, exp, assign } op;
};
struct Word {
  Word() : num{0, 0}, type(WordType::Number) {}
  Word(Token &&t) : tok(std::move(t)), type{WordType::Token} {}
  Word(Number &&t) : num(std::move(t)), type{WordType::Number} {}
  Word(Constant &&t) : con(std::move(t)), type{WordType::Constant} {}
  Word(Variable &&t) : var(std::move(t)), type{WordType::Variable} {}
  Word(Unary &&t) : un(std::move(t)), type{WordType::Unary} {}
  Word(Binary &&t) : bin(std::move(t)), type{WordType::Binary} {}
  Word(std::string_view word)
      : tok{word.data(), word.size()}, type{WordType::Token} {}
  Word(const Word &w) : type(w.type) {
    switch (type) {
    case WordType::Token:
      tok = w.tok;
      break;
    case WordType::Number:
      num = w.num;
      break;
    case WordType::Constant:
      con = w.con;
      break;
    case WordType::Variable:
      var = w.var;
      break;
    case WordType::Unary:
      un = w.un;
      break;
    case WordType::Binary:
      bin = w.bin;
      break;
    }
  }

  Word &operator=(const Word &other) {
    this->~Word();
    type = other.type;
    switch (type) {
    case WordType::Number:
      num = other.num;
      break;
    case WordType::Constant:
      con = other.con;
      break;
    case WordType::Unary:
      un = other.un;
      break;
    case WordType::Binary:
      bin = other.bin;
      break;
    case WordType::Token:
      tok = other.tok;
      break;
    case WordType::Variable:
      var = other.var;
      break;
    }
    return *this;
  }

  ~Word() noexcept {
    switch (type) {
    case WordType::Token:
      destroy(tok);
      break;
    case WordType::Number:
      destroy(num);
      break;
    case WordType::Constant:
      destroy(con);
      break;
    case WordType::Variable:
      destroy(var);
      break;
    case WordType::Unary:
      destroy(un);
      break;
    case WordType::Binary:
      destroy(bin);
      break;
    }
  }

  union {
    Token tok;
    Number num;
    Constant con;
    Variable var;
    Unary un;
    Binary bin;
  };

  WordType type;

private:
  template <typename T> constexpr void destroy(T &v) noexcept { v.~T(); }
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
    return formatter<string_view>::format(std::string_view(t.s, t.size), ctx);
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
    switch (c.types) {
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
    return fmt::format_to(ctx.out(), "({})", std::string_view(v.s, v.size));
  }
};

template <> struct fmt::formatter<fcalc::Unary> : formatter<std::string_view> {
  constexpr auto format(const fcalc::Unary &u, format_context &ctx) const
      -> format_context::iterator {
    std::string_view result = "unknown";
    switch (u.op) {
    case fcalc::Unary::Ops::minus:
      result = "√";
      break;
    case fcalc::Unary::Ops::sqrt:
      result = "sqrt";
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
    case mult:
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