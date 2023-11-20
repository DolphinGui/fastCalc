#include <benchmark/benchmark.h>
#include <fast_calc/fcalc.hpp>
#include <test/calc.hpp>

constexpr const char *unparsed =
    "11422442284^504/"
    "23185203139+tautautau-3313-tautautauipi-422441202917-tautautaupii^"
    "273943292212/287^i+281823413915^46214/46391825/"
    "5018306*pipipi+itaupitautau^tauipi*425154107/i+8342621+214220+itaupi/"
    "tau*23316202026*ipitaupipi-tautautauipi+39264724/444219^ipitauii-2526449/"
    "ipii*iitautaui^5010";

void fcalc_bench(benchmark::State &s) {
  for (auto _ : s) {
    auto n = fcalc::tokenize(unparsed);
    fcalc::parse(n);
    benchmark::DoNotOptimize(n);
  }
}
BENCHMARK(fcalc_bench);

void calc_bench(benchmark::State &s) {
  for (auto _ : s) {
    auto n = calc::tokenize(unparsed);
    auto r = calc::parse(std::move(n));
    benchmark::DoNotOptimize(r);
  }
}
BENCHMARK(calc_bench);

BENCHMARK_MAIN();