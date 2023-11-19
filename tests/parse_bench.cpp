#include <benchmark/benchmark.h>
#include <test/calc.hpp>
#include <fast_calc/fcalc.hpp>

void string(benchmark::State &s) {
  std::string a;
  for (auto _ : s) {
    benchmark::DoNotOptimize(a.length());
  }
}
BENCHMARK(string);

BENCHMARK_MAIN();