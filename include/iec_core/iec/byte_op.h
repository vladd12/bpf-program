#pragma once

#include <cstdint>

using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

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
