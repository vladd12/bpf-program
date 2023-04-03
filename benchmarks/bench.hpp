#pragma once

#include <benchmark/benchmark.h>

/* * * * * * * * * * * * * Documentation * * * * * * * * * * * * * * * *
 * - https://github.com/google/benchmark/blob/main/README.md           *
 * - https://github.com/google/benchmark/blob/main/docs/user_guide.md  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/// \brief String operations benchmarks.
namespace bm_str
{
void BM_StringCreation(benchmark::State &state);
void BM_StringCopy(benchmark::State &state);
void BM_StringCompare(benchmark::State &state);
};

namespace bm_sys
{

};

/// \brief Standard operations benchmarks.
namespace bm_std
{
void BM_printf(benchmark::State &state);
void BM_printf_settings(benchmark::internal::Benchmark *bench);
};

/// \brief Memory operations benchmarks.
namespace bm_mem
{
void BM_memcpy(benchmark::State &state);
};

namespace bm_file
{

};
