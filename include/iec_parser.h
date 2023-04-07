#pragma once

#include <byte_op.h>
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
    byte *mData;
    std::uint16_t mSize;

    bool applyOffset(std::uint16_t offset);
    bool verifySize(std::uint16_t size);

    byte readByte();
    word readWord();
    dword readDword();
    qword readQword();

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
        }
        else
            length = firstByte;
        return length;
    }

    void parsePDU()
    {
        [[maybe_unused]] auto head = PDUHeader { readWord(), readWord(), readWord(), readWord() };
        [[maybe_unused]] auto len = head.length;
    }

public:
    explicit IecParser();
    explicit IecParser(byte *data, const std::uint16_t size);
    bool update(byte *data, const std::uint16_t size);

    SeqASDU parse()
    {
        SeqASDU seq;
        applyOffset(sizeof(ether_header));
        parsePDU();
        return seq;
    }
};

}
