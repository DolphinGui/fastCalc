#pragma once

#include <cstddef>
#include <cstring>
#include <fmt/core.h>
#include <new>
#include <string_view>

namespace fcalc {
class SmolString {
public:
  constexpr SmolString() = default;
  constexpr SmolString(std::string_view sv) {
    if (is_buffer(sv.size())) {
      std::memcpy(buffer, sv.data(), sv.size());
    } else {
      str = static_cast<char *>(::operator new(sv.size()));
      std::memcpy(str, sv.data(), sv.size());
    }
    _size = sv.size();
  }

  constexpr SmolString(SmolString const &s) {
    if (s.is_buffer()) {
      std::memcpy(buffer, s.buffer, sizeof(buffer));
    } else {
      str = static_cast<char *>(::operator new(s.size()));
      std::memcpy(str, s.str, s.size());
    }
    _size = s.size();
  }

  constexpr friend void swap(SmolString &a, SmolString &b) {
    std::swap(a.str, b.str);
    std::swap(a._size, b._size);
  }

  constexpr SmolString(SmolString &&s) noexcept : SmolString() {
    swap(*this, s);
  }

  constexpr SmolString &operator=(SmolString s) {
    swap(*this, s);
    return *this;
  }

  constexpr const char *data() const noexcept {
    if (is_buffer())
      return buffer;
    return str;
  }
  char *data() noexcept {
    if (is_buffer())
      return buffer;
    return str;
  }

  constexpr size_t size() const noexcept { return _size; }

  constexpr std::string_view view() const noexcept {
    return std::string_view(data(), size());
  }

  constexpr bool operator==(SmolString const &other) const noexcept {
    if (size() != other.size())
      return false;
    if (is_buffer())
      return std::memcmp(buffer, other.buffer, sizeof(buffer));
    return std::memcmp(str, other.str, size());
  }

  constexpr operator std::string_view() const noexcept { return view(); }

  constexpr ~SmolString() {
    if (!is_buffer()) {
      ::operator delete(str);
    }
  }

private:
  union {
    char *str{};
    char buffer[sizeof(str)];
  };
  size_t _size{};
  bool is_buffer() const noexcept { return size() < buf_limit; }
  static bool is_buffer(size_t size) noexcept { return size < buf_limit; }
  constexpr static auto buf_limit = sizeof(str) + 1;
};
} // namespace fcalc