#pragma once

#include <cstdint>

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;
using qword = std::uint64_t;

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

template <class In, class Out> //
constexpr Out make(const In lhs, const In rhs)
{
    return Out(lhs << sizeof(In) * 8 | rhs);
}

template <> //
constexpr qword make(const dword lhs, const dword rhs)
{
    auto tmp = qword(lhs << 31);
    tmp = tmp << 1;
    return qword(tmp | rhs);
}

constexpr inline word makeword(const byte lhs, const byte rhs)
{
    return make<byte, word>(lhs, rhs);
}

constexpr inline dword makedword(const word lhs, const word rhs)
{
    return make<word, dword>(lhs, rhs);
}

constexpr inline qword makeqword(const dword lhs, const dword rhs)
{
    return make<dword, qword>(lhs, rhs);
}

/* Using byteswap built-in functions given by gcc.
 * Translating functions to the single processor instruction.
 * Example for gcc 10.2: https://godbolt.org/z/1csGGeE16
 */

ui16 byteswap(ui16 x);
ui32 byteswap(ui32 x);
ui64 byteswap(ui64 x);

}
