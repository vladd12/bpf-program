#pragma once

#include <cstdint>

namespace iec
{

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;
using qword = std::uint64_t;

template <typename T> struct value
{
    constexpr static auto size = sizeof(T);
};

template <class In, class Out> //
constexpr Out make(const In lhs, const In rhs)
{
    return Out(lhs << sizeof(In) * 8 | rhs);
}

constexpr inline word makeword(const byte lhs, const byte rhs)
{
    return make<byte, word>(lhs, rhs);
}

constexpr inline dword makedword(const word lhs, const word rhs)
{
    return make<word, dword>(lhs, rhs);
}

///////////////////////////////////////////////////////////////////////////////

struct ASDU
{
    std::int16_t smpCnt;
};

struct seqASDU
{
    std::uint8_t count;
    ASDU *data;
};

class IecParser
{
private:
    byte *mData;
    std::uint16_t mSize;
    word prevSmpCnt;

public:
    explicit IecParser(byte *data, const std::uint16_t size) : mData(data), mSize(size)
    {
    }

    void parse()
    {
        ;
    }
};

}
