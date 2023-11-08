#include "fcalc.hpp"
#include "re.hpp"

#include <fmt/ranges.h>
#include <pcre2.h>

namespace fcalc {
std::vector<Word> tokenize(std::string_view input) {
  std::vector<Word> result;
  auto p = re::Pattern(R"((\d+)|[+\-*/^()ie=]|pi|tau|\S)",
                       re::Options::ucp | re::Options::utf);
  result.reserve(20);
  for (auto match : p.match(input)) {
    result.push_back(Word(match));
  }
  return result;
}
void parse(std::span<Word> s);
void resolve(std::span<Word> s);
} // namespace fcalc