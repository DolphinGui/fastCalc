#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace fcalc {
struct Location {
  uint32_t column, line;
};
enum struct WordType : uint8_t {
  Token,
  Number,
  Constant,
  Variable,
  Unary,
  Binary,
  OpenParen,
  CloseParen,
};
struct Token {
  std::string s;
};
struct Number {
  int64_t num, den;
};
struct Constant {
  enum struct types { pi, e, tau, i };
};
struct Variable {
  std::string s;
};
struct Unary {
  enum struct types { minus, sqrt };
};
struct Binary {
  enum struct types { add, sub, mult, div, exp, assign };
};
struct OpenParen {};
struct CloseParen {};
struct Word {
  Word(std::string_view word, Location l)
      : tok{.s = std::string(word)}, loc{l}, type{WordType::Token} {}
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
    case WordType::OpenParen:
      destroy(open);
      break;
    case WordType::CloseParen:
      destroy(close);
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
    OpenParen open;
    CloseParen close;
  };
  Location loc;
  WordType type;

private:
  template <typename T> constexpr void destroy(T &v) noexcept { v.~T(); }
};
std::vector<Word> tokenize(std::string_view);
void parse(std::span<Word> s);
void resolve(std::span<Word> s);
}; // namespace fcalc