#pragma once

#include <cstdint>
#include <cstring>
#include <new>
#include <string_view>

namespace fcalc {
class SmolString {
  union {
    char *str{};
    char buffer[8];
  };
  uint8_t size{};
  SmolString() = default;
  SmolString(std::string_view sv) {
    if (sv.size() < 8) {
      std::memcpy(buffer, sv.data(), sv.size());
      size = sv.size();
    } else {
      str = static_cast<char *>(::operator new(sv.size()));
    }
  }
};
} // namespace fcalc