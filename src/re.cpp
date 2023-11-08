#include "re.hpp"

#include <fmt/core.h>
#include <pcre2.h>
#include <stdexcept>
#include <utility>

namespace {
void rc_err(int rc) {
  unsigned char message[121]{};
  pcre2_get_error_message(rc, message, sizeof(message));
  throw std::runtime_error(fmt::format("Pattern failed to match: {}",
                                       reinterpret_cast<char *>(message)));
}
} // namespace

namespace re {
namespace detail {} // namespace detail

Matches::~Matches() {
  pcre2_match_data_free(static_cast<pcre2_match_data *>(internal));
}

MatchIterator Matches::begin() {
  auto result = MatchIterator(*this, nullptr, nullptr);
  result.match(true);
  return result;
}

MatchIterator::reference MatchIterator::operator*() const {
  if (start == nullptr)
    throw std::runtime_error("Pattern had no match, attempted to dereference");
  return source->string.substr(start[0], start[1] - start[0]);
}

// this must be called first bc
void MatchIterator::match(bool is_first) {
  size_t offset = is_first ? 0 : start[1];
  auto rc = pcre2_match(
      static_cast<pcre2_code *>(source->pattern),
      reinterpret_cast<const unsigned char *>(source->string.data()),
      source->string.size(), offset, {},
      static_cast<pcre2_match_data *>(source->internal), nullptr);
  if (is_first) {
    start = pcre2_get_ovector_pointer(
        static_cast<pcre2_match_data *>(source->internal));
    end = start + rc;
  }
  if (rc == PCRE2_ERROR_NOMATCH) {
    start = nullptr;
    end = nullptr;
  } else if (rc < 0) {
    rc_err(rc);
  }
}

// this basically always returns true
bool MatchIterator::operator==(const MatchIterator &other) const {
  return other.source == source;
}

MatchIterator::MatchIterator(const MatchIterator &other)
    : start{other.start}, end{other.end}, source(other.source) {}

MatchEnd Matches::end() { return MatchEnd{}; }

#define pattern_ptr static_cast<pcre2_code *>(internal)

Pattern::Pattern(std::string_view pattern, Options options) {
  int errcode{};
  size_t erroffset{};
  internal = pcre2_compile(
      reinterpret_cast<const unsigned char *>(pattern.data()), pattern.size(),
      std::to_underlying(options), &errcode, &erroffset, nullptr);
  if (!internal) {
    unsigned char message[121]{};
    pcre2_get_error_message(errcode, message, sizeof(message));
    std::runtime_error(fmt::format("Pattern failed to compile at offset {}: {}",
                                   erroffset,
                                   reinterpret_cast<char *>(message)));
  }
}
Pattern::~Pattern() { pcre2_code_free(pattern_ptr); }

Matches Pattern::match(std::string_view str) {
  auto data = pcre2_match_data_create_from_pattern(
      static_cast<pcre2_code *>(internal), nullptr);
  if (!data)
    throw std::runtime_error("Match data failed to construct.");
  return Matches(data, internal, str);
}

} // namespace re