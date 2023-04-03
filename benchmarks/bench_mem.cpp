#include "bench.hpp"

#include <cstring>

void bm_mem::BM_memcpy(benchmark::State &state)
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
BENCHMARK(bm_mem::BM_memcpy)->RangeMultiplier(2)->Range(8, 8 << 10);
