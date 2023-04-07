#pragma once

#include <byte_op.h>
#include <cassert>
#include <net/ethernet.h>

namespace iec
{
struct PDUHeader
{
    ui16 appID;
    ui16 length;
    ui16 res1;
    ui16 res2;
} __attribute__((__packed__));

union DataFrame {
    struct __attribute__((__packed__))
    {
        ui32 instMagI;
        ui16 quality;
        union bitfield {
            struct __attribute__((__packed__))
            {
                ui16 reserved : 2;
                ui16 der : 1;
                ui16 opB : 1;
                ui16 test : 1;
                ui16 src : 1;
                ui16 dQual : 8;
                ui16 valid : 2;
            } data;
            ui16 data_;
        } bitset;
    } data;
    ui64 data_;
};

constexpr auto framesPerASDU = 8;

struct ASDU
{
    ui16 smpCnt;
    ui32 confRev;
    ui8 smpSync;
    DataFrame data[framesPerASDU];
};

struct SeqASDU
{
    ui8 count;
    ASDU *data;
};

class IecParser
{
private:
    ui8 *mData;
    ui16 mSize;

    bool applyOffset(ui16 offset);
    bool verifySize(ui16 size);

    ui8 readByte();
    ui16 readWord();
    ui32 readDword();
    ui64 readQword();

    template <typename T> T read()
    {
        T *val = reinterpret_cast<T *>(mData);
        applyOffset(sizeof(T));
        return *val;
    }

    ui32 parseAsnLength()
    {
        auto firstByte = readByte();
        ui32 length = 0;
        if (firstByte & 0x80)
        {
            auto asnLength = firstByte & 0x7f;
            if (asnLength > 4)
                return 0;
            while (asnLength > 0)
            {
                length += readByte();
                asnLength--;
                if (asnLength > 0)
                    length = length << 8;
            }
            return length;
        }
        else
            return firstByte;
    }

    bool parsePDU(SeqASDU &seq)
    {
        constexpr ui8 stdPduId = 0x60;
        constexpr ui8 stdNoAsduId = 0x80;

        auto head = PDUHeader { readWord(), readWord(), readWord(), readWord() };
        ui32 length = head.length - sizeof(head);
        if (!verifySize(length))
            return false;
        // reading savPDU
        auto pduId = readByte();
        if (pduId != stdPduId)
            return false;
        length = parseAsnLength();
        if (!verifySize(length))
            return false;

        // reading noASDU
        auto noAsduId = readByte();
        if (noAsduId != stdNoAsduId)
            return false;
        length = readByte();
        if (length != 1)
            return false;
        auto seqCount = readByte();
        if (seqCount > 16)
            return false;

        seq.count = seqCount;
        return true;
    }

    bool parseSequence([[maybe_unused]] SeqASDU &seq)
    {
        auto seqAsduId = readByte();
        if (seqAsduId != 0xa2)
            return false;
        ui32 length = parseAsnLength();
        if (!verifySize(length))
            return false;

        return true;
    }

public:
    explicit IecParser();
    explicit IecParser(ui8 *data, const ui16 size);
    bool update(ui8 *data, const ui16 size);

    SeqASDU parse()
    {
        SeqASDU seq;
        applyOffset(sizeof(ether_header));
        auto status = parsePDU(seq);
        assert(status);
        status = parseSequence(seq);
        assert(status);
        return seq;
    }
};

}
