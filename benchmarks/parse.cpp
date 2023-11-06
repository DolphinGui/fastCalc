#include <benchmark/benchmark.h>

void string(benchmark::State &s) {
  std::string a;
  for (auto _ : s) {
    benchmark::DoNotOptimize(a.length());
  }
}
BENCHMARK(string);

BENCHMARK_MAIN();