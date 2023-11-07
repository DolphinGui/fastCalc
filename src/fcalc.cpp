#include "fcalc.hpp"
#include "re.hpp"

#include <cstddef>
#include <fmt/ranges.h>
#include <pcre2.h>

namespace fcalc {
std::vector<Word> tokenize(std::string_view input) {
  std::vector<Word> result;
  auto p = re::Pattern(R"((\d+)|[+\-*/^()ie=]|pi|tau|\X)",
                       re::Options::ucp | re::Options::utf);
  for (auto match : p.match(input)) {
    fmt::print("match: {}\n", match);
  }
  return result;
}
void parse(std::span<Word> s);
void resolve(std::span<Word> s);
} // namespace fcalc