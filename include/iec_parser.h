#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <net/ethernet.h>

namespace iec
{

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;
using qword = std::uint64_t;

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

///////////////////////////////////////////////////////////////////////////////

union dataFrame {
    struct __attribute__((__packed__))
    {
        std::uint32_t ampInstMagI;
        std::uint16_t ampQ;
        union bitfield {
            struct __attribute__((__packed__))
            {
                std::uint16_t reserved : 2;
                std::uint16_t der : 1;
                std::uint16_t opB : 1;
                std::uint16_t test : 1;
                std::uint16_t src : 1;
                std::uint16_t dQual : 8;
                std::uint16_t valid : 2;
            } data;
            std::uint16_t data_;
        } bitset;
    } data;
    std::uint64_t data_;
};

struct ASDU
{
    std::int16_t smpCnt;
    std::uint32_t confRev;
    std::uint8_t smpSync;
    std::uint8_t size; // usually 0x40
    dataFrame *data;
};

struct seqASDU
{
    std::uint8_t count;
    ASDU *data;
};

enum ParseMode : bool
{
    P80 = false,
    P256 = true
};

class IecParser
{
private:
    byte *mData;
    std::uint16_t mSize;
    word prevSmpCnt;

    bool applyOffset(std::uint16_t offset)
    {
        if (mSize - offset > 0)
        {
            mData += offset;
            mSize -= offset;
            return true;
        }
        return false;
    }

    byte readByte()
    {
        auto _byte = *mData;
        mData++;
        mSize--;
        return _byte;
    }

    word readWord()
    {
        auto _word = makeword(*mData, *(mData + 1));
        mData += sizeof(word);
        mSize -= sizeof(word);
        return _word;
    }

    dword readDword()
    {
        auto _dword = makedword(makeword(*mData, *(mData + 1)), makeword(*(mData + 2), *(mData + 3)));
        mData += sizeof(dword);
        mSize -= sizeof(dword);
        return _dword;
    }

    qword readQword()
    {
        return makeqword(readDword(), readDword());
    }

    bool verifySize(ParseMode mode)
    {
        if (mode == ParseMode::P80)
        {
            auto len = readByte();
            return len == mSize;
        }
        else if (mode == ParseMode::P256)
        {
            auto len = readWord();
            return len == mSize;
        }
        else
        {
            return false;
        }
    }

    void parse80(seqASDU &seq)
    {
        ;
    }

    void parse256(seqASDU &seq)
    {
        ;
    }

    ParseMode defineParseMode()
    {
        auto pduId = readWord();
        if (pduId == 0x6082)
            return ParseMode::P256;
        else
            return ParseMode::P80;
    }

public:
    explicit IecParser() : mData(nullptr), mSize(0)
    {
    }

    explicit IecParser(byte *data, const std::uint16_t size) : mData(data), mSize(size)
    {
    }

    bool update(byte *data, const std::uint16_t size)
    {
        if (data != nullptr && size > 0)
        {
            mData = data;
            mSize = size;
            return true;
        }
        return false;
    }

    seqASDU parse()
    {
        constexpr static std::uint16_t iecHeaderSize = 8;
        [[maybe_unused]] seqASDU seq;
        applyOffset(sizeof(ether_header) + iecHeaderSize);
        auto parseMode = defineParseMode();
        if (parseMode == ParseMode::P80)
        {
            // Step back
            mData--;
            mSize++;
            assert(verifySize(parseMode));
            parse80(seq);
        }
        else
        {
            assert(verifySize(parseMode));
            parse256(seq);
        }
        return seq;
    }
};

}
