#pragma once

#include <cstdint>

#ifdef __cpp_lib_hardware_interference_size
#include <new>
#endif

using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

namespace details
{
#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
constexpr std::size_t hardware_constructive_interference_size = 64;
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif
} // namespace details

namespace bytes
{
/* Using byteswap built-in functions given by gcc.
 * Translating functions to the single processor instruction.
 * Example for gcc 10.2: https://godbolt.org/z/1csGGeE16
 */

u16 byteswap(u16 x);
u32 byteswap(u32 x);
u64 byteswap(u64 x);

}
