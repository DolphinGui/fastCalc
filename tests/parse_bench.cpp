#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <fast_calc/fcalc.hpp>
#include <fmt/core.h>
#include <gperftools/malloc_hook.h>
#include <random>
#include <test/calc.hpp>
#include <type_traits>
#include <vector>

benchmark::IterationCount g_num_new = 0;
benchmark::IterationCount g_sum_size_new = 0;
auto new_hook = [](const void *, size_t s) {
  benchmark::IterationCount size = s;
  ++g_num_new;
  g_sum_size_new += size;
};

#define BEFORE_TEST()                                                          \
  benchmark::IterationCount num_new = g_num_new;                               \
  benchmark::IterationCount sum_size_new = g_sum_size_new;                     \
  MallocHook::AddNewHook(new_hook)

#define AFTER_TEST()                                                           \
  MallocHook::RemoveNewHook(new_hook);                                         \
  auto iter = double(state.iterations());                                      \
  state.counters["allocs"] = (g_num_new - num_new) / iter;                     \
  state.counters["avg alloc"] =                                                \
      (g_sum_size_new - sum_size_new) / double(g_num_new - num_new);

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

template <typename T> using int_dist = std::uniform_int_distribution<T>;
namespace c_gen {

calc::WordPtr ran_op(auto &r) {
  int_dist<uint32_t> t(1, 4);
  return calc::WordPtr(new calc::Binary([&]() {
    switch (t(r)) {
      using enum calc::Binary::Ops;
    case 0:
      return calc::Binary(assign);
    case 1:
      return calc::Binary(add);
    case 2:
      return calc::Binary(sub);
    case 3:
      return calc::Binary(mul);
    case 4:
      return calc::Binary(div);
    case 5:
      return calc::Binary(exp);
    default:
      return calc::Binary(add);
    }
  }()));
}

auto ran_val(auto &r) {
  auto ptr = [](auto &&a) {
    using T = std::remove_cvref_t<decltype(a)>;
    return calc::WordPtr(new T(std::move(a)));
  };
  int_dist<uint32_t> rbool(0, 1);
  if (rbool(r)) {
    int_dist<uint32_t> i(1, 100);
    if (rbool(r)) {
      return ptr(calc::Number(i(r)));
    } else {
      return ptr(calc::Number(i(r), i(r)));
    }
  } else {
    int_dist<uint32_t> t(1, 4);
    switch (t(r)) {
      using enum calc::Constant::Types;
    case 1:
      return ptr(calc::Constant(pi));
    case 2:
      return ptr(calc::Constant(e));
    case 3:
      return ptr(calc::Constant(tau));
    case 4:
      return ptr(calc::Constant(i));
    default:
      return ptr(calc::Constant(pi));
    }
  }
}
auto gen_exp(uint32_t terms, uint32_t term_size) {
  std::vector<calc::WordPtr> w;
  w.reserve(terms * term_size);
  std::random_device _r;
  std::default_random_engine e(_r());
  auto gen_term = [&]() {
    for (uint32_t i = 0; i != term_size; ++i) {
      w.push_back(ran_val(e));
    }
  };
  for (uint32_t i = 0; i != terms - 1; ++i) {
    gen_term();
    w.push_back(ran_op(e));
  }
  gen_term();
  return w;
}
} // namespace c_gen

namespace f_gen {

fcalc::Word ran_op(auto &r) {
  int_dist<uint32_t> t(1, 5);
  using enum fcalc::Binary::Ops;
  return fcalc::Binary(fcalc::Binary::Ops(t(r)));
}

fcalc::Word ran_val(auto &r) {
  int_dist<uint32_t> rbool(0, 1);
  if (rbool(r)) {
    int_dist<uint32_t> i(1, 100);
    return fcalc::Number(i(r));
  } else {
    int_dist<uint32_t> t(1, 4);
    switch (t(r)) {
      using enum fcalc::Constant::Types;
    case 1:
      return fcalc::Constant(pi);
    case 2:
      return fcalc::Constant(e);
    case 3:
      return fcalc::Constant(tau);
    case 4:
      return fcalc::Constant(i);
    default:
      return fcalc::Constant(pi);
    }
  }
}
auto gen_exp(uint32_t terms, uint32_t term_size) {
  std::vector<fcalc::Word> w;
  w.reserve(terms * term_size);
  std::random_device _r;
  std::default_random_engine r(_r());
  auto gen_term = [&]() {
    for (uint32_t i = 0; i != term_size; ++i) {
      w.push_back(ran_val(r));
    }
  };
  for (uint32_t i = 0; i != terms - 1; ++i) {
    gen_term();
    w.push_back(ran_op(r));
  }
  gen_term();
  return w;
}
} // namespace f_gen
} // namespace

void fcalc_bench(benchmark::State &state) {
  BEFORE_TEST();
  std::string expression = gen_expression(state.range(0), state.range(1));
  for (auto _ : state) {
    auto n = fcalc::tokenize(expression);
    fcalc::parse(n);
    benchmark::DoNotOptimize(n);
    benchmark::ClobberMemory();
  }
  state.SetComplexityN(expression.size());
  state.counters["data size"] = expression.size();
  state.counters["efficiency"] =
      (g_sum_size_new - sum_size_new) / double(expression.size());
  AFTER_TEST();
}
BENCHMARK(fcalc_bench)->Ranges({{8 << 5, 8 << 10}, {2, 8}})->Complexity();

void ccalc_bench(benchmark::State &state) {
  BEFORE_TEST();
  std::string expression = gen_expression(state.range(0), state.range(1));
  for (auto _ : state) {
    auto n = calc::tokenize(expression);
    auto r = calc::parse(std::move(n));
    benchmark::DoNotOptimize(r);
    benchmark::ClobberMemory();
  }
  state.SetComplexityN(expression.size());
  state.counters["data size"] = expression.size();
  state.counters["efficiency"] =
      (g_sum_size_new - sum_size_new) / double(expression.size());
  AFTER_TEST();
}
BENCHMARK(ccalc_bench)->Ranges({{8 << 5, 8 << 10}, {2, 8}})->Complexity();

void fcalc_parse(benchmark::State &state) {
  BEFORE_TEST();
  for (auto _ : state) {
    state.PauseTiming();
    auto a = f_gen::gen_exp(state.range(0), state.range(1));
    state.ResumeTiming();
    fcalc::parse(a);
    benchmark::DoNotOptimize(a);
  }
  state.SetComplexityN(state.range(0) * state.range(1));
  state.counters["data num"] = state.range(0) * state.range(1) * 2;
  state.counters["byte/word"] =
      (g_sum_size_new - sum_size_new) / state.counters["data num"];
  AFTER_TEST();
}
BENCHMARK(fcalc_parse)->Ranges({{8 << 5, 8 << 7}, {2, 8}})->Complexity();

void ccalc_parse(benchmark::State &state) {
  BEFORE_TEST();
  for (auto _ : state) {
    state.PauseTiming();
    auto expression = c_gen::gen_exp(state.range(0), state.range(1));
    state.ResumeTiming();
    auto r = calc::parse(std::move(expression));
    benchmark::DoNotOptimize(r);
  }
  state.SetComplexityN(state.range(0) * state.range(1));
  state.counters["data num"] = state.range(0) * state.range(1) * 2;
  state.counters["byte/word"] =
      (g_sum_size_new - sum_size_new) / state.counters["data num"];
  AFTER_TEST();
}
BENCHMARK(ccalc_parse)
    ->Ranges({{8 << 5, 8 << 7}, {2, 8}})
    ->Complexity(benchmark::oNLogN);

BENCHMARK_MAIN();