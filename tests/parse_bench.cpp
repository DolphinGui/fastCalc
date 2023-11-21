#include <benchmark/benchmark.h>
#include <cstdint>
#include <fast_calc/fcalc.hpp>
#include <random>
#include <test/calc.hpp>

namespace {
const char *ops[] = {"-", "+", "*", "/", "^"};
const char *rand_op(auto &r) {
  std::uniform_int_distribution<uint32_t> rand(0, 4);
  return ops[rand(r)];
}
const char *constants[] = {"pi", "tau", "i"};
const char *rand_con(auto &r) {
  std::uniform_int_distribution<uint32_t> rand(0, 2);
  return constants[rand(r)];
}
void rand_term(std::string &expression, auto &r, uint32_t term_max) {
  std::uniform_int_distribution<uint32_t> rand(1, term_max);
  uint32_t term_length = rand(r);
  std::uniform_int_distribution<uint32_t> rbool(0, 1);
  for (uint32_t i = 0; i != term_length; ++i) {
    if (rbool(r)) {
      expression.append(rand_con(r));
    } else {
      std::uniform_int_distribution<uint32_t> i(1, 50);
      expression.append(std::to_string(i(r)));
    }
  }
}

std::string gen_expression(uint32_t terms, uint32_t term_max) {
  std::random_device r;
  std::default_random_engine e(r());
  std::string expression;
  expression.reserve(3 * term_max / 2 * terms);
  for (uint32_t i = 0; i != terms - 1; i++) {
    rand_term(expression, e, term_max);
    expression.append(rand_op(e));
  }
  rand_term(expression, e, term_max);
  return expression;
}

} // namespace

void fcalc_bench(benchmark::State &s) {
  std::string expression = gen_expression(s.range(0), s.range(1));
  for (auto _ : s) {
    auto n = fcalc::tokenize(expression);
    fcalc::parse(n);
    benchmark::DoNotOptimize(n);
  }
  s.SetComplexityN(expression.size());
}
BENCHMARK(fcalc_bench)->Ranges({{8, 8 << 10}, {2, 10}})->Complexity();

void calc_bench(benchmark::State &s) {
  std::string expression = gen_expression(s.range(0), s.range(1));
  for (auto _ : s) {
    auto n = calc::tokenize(expression);
    auto r = calc::parse(std::move(n));
    benchmark::DoNotOptimize(r);
  }
  s.SetComplexityN(expression.size());
}
BENCHMARK(calc_bench)->Ranges({{8, 8 << 10}, {2, 10}})->Complexity();

BENCHMARK_MAIN();