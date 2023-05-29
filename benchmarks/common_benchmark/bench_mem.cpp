#include "bench.hpp"

#include <cstring>

namespace bm_mem
{

void BM_memcpy(benchmark::State &state)
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

void BM_malloc(benchmark::State &state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        auto data = malloc(state.range(0));
        free(data);
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
BENCHMARK(BM_malloc)->RangeMultiplier(2)->Range(32, 4096);

void BM_new(benchmark::State &state)
{
    while (state.KeepRunning())
    {
        auto data = new std::uint8_t[state.range(0)];
        delete[] data;
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
BENCHMARK(BM_new)->RangeMultiplier(2)->Range(32, 4096);

}
