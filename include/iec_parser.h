#pragma once

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

union DataFrame {
    struct __attribute__((__packed__))
    {
        std::uint32_t instMagI;
        std::uint16_t quality;
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

constexpr auto framesPerASDU = 8;

struct ASDU
{
    std::int16_t smpCnt;
    std::uint32_t confRev;
    std::uint8_t smpSync;
    DataFrame data[framesPerASDU];
};

struct SeqASDU
{
    std::uint8_t count;
    ASDU *data;
};

class IecParser
{
private:
    byte *mData;
    std::uint16_t mSize;
    // word prevSmpCnt;

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
        applyOffset(sizeof(byte));
        return _byte;
    }

    word readWord()
    {
        auto _word = makeword(*mData, *(mData + 1));
        applyOffset(sizeof(word));
        return _word;
    }

    dword readDword()
    {
        auto _dword = makedword(makeword(*mData, *(mData + 1)), makeword(*(mData + 2), *(mData + 3)));
        applyOffset(sizeof(dword));
        return _dword;
    }

    qword readQword()
    {
        auto _dword1 = makedword(makeword(*mData, *(mData + 1)), makeword(*(mData + 2), *(mData + 3)));
        auto _dword2 = makedword(makeword(*(mData + 4), *(mData + 5)), makeword(*(mData + 6), *(mData + 7)));
        applyOffset(sizeof(qword));
        return makeqword(_dword1, _dword2);
    }

    bool verifySize(std::uint16_t size)
    {
        return size == mSize;
    }

    //    void parse80(SeqASDU &seq)
    //    {
    //        ;
    //    }

    //    void parse256(SeqASDU &seq)
    //    {
    //        ;
    //    }

public:
    explicit IecParser();
    explicit IecParser(byte *data, const std::uint16_t size);
    bool update(byte *data, const std::uint16_t size);

    int parseAsnLength()
    {
        int LL, len = 0, ln;

        if (*mData & 0x80)
        {
            LL = *mData & 0x7f;
            if (LL > 4)
                return -2;
            mData++;
            ln = LL + 1;
        }
        else
        {
            LL = 1;
            ln = LL;
        }

        while (LL > 0)
        {
            len = len + *mData;
            LL--;
            if (LL)
            {
                len = len << 8;
                mData++;
            }
        }
        return ln;
    }

    SeqASDU parse()
    {
        constexpr static std::uint16_t iecHeaderSize = 8;
        SeqASDU seq;
        applyOffset(sizeof(ether_header) + iecHeaderSize);
        //        auto parseMode = defineParseMode();
        //        if (parseMode == ParseMode::P80)
        //        {
        //            // Step back
        //            mData--;
        //            mSize++;
        //            GetAsnLen();
        //            assert(verifySize(parseMode));
        //            parse80(seq);
        //        }
        //        else
        //        {
        //            mData--;
        //            mSize++;
        //            GetAsnLen();
        //            assert(verifySize(parseMode));
        //            parse256(seq);
        //        }
        return seq;
    }
};

}
