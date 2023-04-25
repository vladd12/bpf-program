#include "bench.hpp"

#include <string>

namespace bm_str
{

void BM_StringCreation(benchmark::State &state)
{
    for ([[maybe_unused]] auto _ : state)
        std::string empty_string;
}
BENCHMARK(BM_StringCreation);

void BM_StringCopy(benchmark::State &state)
{
    std::string x = "hello";
    for ([[maybe_unused]] auto _ : state)
        std::string copy(x);
}
BENCHMARK(BM_StringCopy);

void BM_StringCompare(benchmark::State &state)
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
BENCHMARK(bm_str::BM_StringCompare)->RangeMultiplier(2)->Range(1 << 10, 1 << 18)->Complexity(benchmark::oN);

}
