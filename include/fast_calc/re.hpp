#pragma once

#include <cstddef>
#include <string_view>
#include <utility>

namespace re {
namespace detail {
typedef void PatternPtr;
typedef void MatchPtr;
} // namespace detail

enum struct Options : unsigned {
  null = 0,
  allow_empty_class = 0x00000001u,
  alt_bsux = 0x00000002u,
  auto_callout = 0x00000004u,
  caseless = 0x00000008u,
  dollar_endonly = 0x00000010u,
  dotall = 0x00000020u,
  dupnames = 0x00000040u,
  extended = 0x00000080u,
  firstline = 0x00000100u,
  match_unset_backref = 0x00000200u,
  multiline = 0x00000400u,
  never_ucp = 0x00000800u,
  never_utf = 0x00001000u,
  no_auto_capture = 0x00002000u,
  no_auto_possess = 0x00004000u,
  no_dotstar_anchor = 0x00008000u,
  no_start_optimize = 0x00010000u,
  ucp = 0x00020000u,
  ungreedy = 0x00040000u,
  utf = 0x00080000u,
  never_backslash_c = 0x00100000u,
  alt_circumflex = 0x00200000u,
  alt_verbnames = 0x00400000u,
  use_offset_limit = 0x00800000u,
  extended_more = 0x01000000u,
  literal = 0x02000000u,
  match_invalid_utf = 0x04000000u,
};
inline Options operator&(Options lhs, Options rhs) {
  return static_cast<Options>(std::to_underlying(lhs) &
                              std::to_underlying(rhs));
}

inline Options &operator&=(Options &lhs, Options rhs) {
  lhs = static_cast<Options>(std::to_underlying(lhs) & std::to_underlying(rhs));
  return lhs;
}

inline Options operator|(Options lhs, Options rhs) {
  return static_cast<Options>(std::to_underlying(lhs) |
                              std::to_underlying(rhs));
}

inline Options &operator|=(Options &lhs, Options rhs) {
  lhs = static_cast<Options>(std::to_underlying(lhs) | std::to_underlying(rhs));
  return lhs;
}

inline Options operator^(Options lhs, Options rhs) {
  return static_cast<Options>(std::to_underlying(lhs) ^
                              std::to_underlying(rhs));
}

inline Options &operator^=(Options &lhs, Options rhs) {
  lhs = static_cast<Options>(std::to_underlying(lhs) ^ std::to_underlying(rhs));
  return lhs;
}

struct MatchEnd {
  MatchEnd() = default;
  ~MatchEnd() = default;
};

struct Matches;
struct MatchIterator {
  typedef std::string value_type;
  typedef std::string_view reference;
  typedef int difference_type;
  typedef std::input_iterator_tag iterator_category;

  MatchIterator() = default;

  reference operator*() const;

  bool operator==(const MatchIterator &) const;
  bool operator==(const MatchEnd &m) const {
    return start == nullptr && end == nullptr;
  }

  inline MatchIterator &operator++() {
    match(false);
    return *this;
  }

  inline MatchIterator operator++(int) {
    this->operator++();
    return *this;
  }

private:
  MatchIterator(Matches &source, size_t *begin, size_t *end)
      : start{begin}, end{end}, source(&source) {}
  MatchIterator(const MatchIterator &);
  size_t *start{}, *end{};
  Matches *source{};
  void match(bool is_first);
  friend struct Matches;
};

struct Matches {
  ~Matches();
  Matches(Matches &&other)
      : internal{other.internal}, pattern(other.pattern), string(other.string) {
    other.internal = nullptr;
    other.pattern = nullptr;
  }
  MatchIterator begin();
  MatchEnd end();

private:
  detail::MatchPtr *internal{};
  detail::PatternPtr *pattern{};
  std::string_view string;
  Matches(detail::MatchPtr *i, detail::PatternPtr *pattern,
          std::string_view string)
      : internal{i}, pattern{pattern}, string{string} {}

  friend struct Pattern;
  friend struct MatchIterator;
};

struct Pattern {
  Pattern(std::string_view pattern, Options options = Options::null);
  ~Pattern();
  Matches match(std::string_view str);

  detail::PatternPtr *internal{};
};
} // namespace re