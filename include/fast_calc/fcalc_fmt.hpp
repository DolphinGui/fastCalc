#pragma once

#include "fcalc.hpp"
#include <fmt/format.h>

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
    return fmt::format_to(ctx.out(), "({})", v.s);
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

template <>
struct fmt::formatter<fcalc::OpenParen> : formatter<std::string_view> {
  constexpr auto format(const fcalc::OpenParen &, format_context &ctx) const
      -> format_context::iterator {
    return formatter<std::string_view>::format("(", ctx);
  }
};

template <>
struct fmt::formatter<fcalc::CloseParen> : formatter<std::string_view> {
  constexpr auto format(const fcalc::CloseParen &, format_context &ctx) const
      -> format_context::iterator {
    return formatter<std::string_view>::format(")", ctx);
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
    case OpenParen:
      return fmt::format_to(ctx.out(), "{}", w.open);
    case CloseParen:
      return fmt::format_to(ctx.out(), "{}", w.close);
    }
    return fmt::format_to(ctx.out(), "?");
  }
};