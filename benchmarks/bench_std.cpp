#include "bench.hpp"

#include <cstdint>
#include <cstdio>

void bm_std::BM_printf(benchmark::State &state)
{
    for ([[maybe_unused]] auto _ : state)
        printf("%04X ", static_cast<std::uint16_t>(state.range(0)));
}

void bm_std::BM_printf_settings(benchmark::internal::Benchmark *bench)
{
    constexpr auto iterations = 5;
    for (int i = 1; i <= iterations; i++)
        bench->Threads(1)->Iterations(i)->RangeMultiplier(2)->Range(8, 8 << 10);
}
BENCHMARK(bm_std::BM_printf)->Apply(bm_std::BM_printf_settings);
