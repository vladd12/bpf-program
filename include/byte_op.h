#pragma once

#include <cstdint>

using i8 = std::int8_t;
using ui8 = std::uint8_t;
using i16 = std::int16_t;
using ui16 = std::uint16_t;
using i32 = std::int32_t;
using ui32 = std::uint32_t;
using i64 = std::int64_t;
using ui64 = std::uint64_t;

namespace bytes
{
/* Using byteswap built-in functions given by gcc.
 * Translating functions to the single processor instruction.
 * Example for gcc 10.2: https://godbolt.org/z/1csGGeE16
 */

ui16 byteswap(ui16 x);
ui32 byteswap(ui32 x);
ui64 byteswap(ui64 x);

}
