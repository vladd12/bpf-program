#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

/* * * * * * * * * * * * * Documentation * * * * * * * * * * * * * * * *
 * - https://github.com/google/benchmark/blob/main/README.md           *
 * - https://github.com/google/benchmark/blob/main/docs/user_guide.md  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void BM_StringCreation(benchmark::State &state)
{
    for ([[maybe_unused]] auto _ : state)
        std::string empty_string;
}
BENCHMARK(BM_StringCreation);

static void BM_StringCopy(benchmark::State &state)
{
    std::string x = "hello";
    for ([[maybe_unused]] auto _ : state)
        std::string copy(x);
}
BENCHMARK(BM_StringCopy);

static void BM_StringCompare(benchmark::State &state)
{
    std::string s1(state.range(0), '-');
    std::string s2(state.range(0), '-');
    for ([[maybe_unused]] auto _ : state)
    {
        auto comparison_result = s1.compare(s2);
        benchmark::DoNotOptimize(comparison_result);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_StringCompare)->RangeMultiplier(2)->Range(1 << 10, 1 << 18)->Complexity(benchmark::oN);

static void BM_printf(benchmark::State &state)
{
    for ([[maybe_unused]] auto _ : state)
        printf("%04X ", static_cast<std::uint16_t>(state.range(0)));
}

static void BM_printf_settings(benchmark::internal::Benchmark *bench)
{
    constexpr auto iterations = 5;
    for (int i = 1; i <= iterations; i++)
        bench->Threads(1)->Iterations(i)->RangeMultiplier(2)->Range(8, 8 << 10);
}
BENCHMARK(BM_printf)->Apply(BM_printf_settings);

static void BM_memcpy(benchmark::State &state)
{
    char *src = new char[state.range(0)];
    char *dst = new char[state.range(0)];
    memset(src, 'x', state.range(0));
    for ([[maybe_unused]] auto _ : state)
        memcpy(dst, src, state.range(0));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
    delete[] src;
    delete[] dst;
}
BENCHMARK(BM_memcpy)->RangeMultiplier(2)->Range(8, 8 << 10);

BENCHMARK_MAIN();
