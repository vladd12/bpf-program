#pragma once

#include <cstdint>

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;
using qword = std::uint64_t;

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

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
// smth
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
// smth else
#endif

}
