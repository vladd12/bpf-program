#pragma once

#include <byte_op.h>
#include <cassert>
#include <cstring>
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
    bool verifySize(ui16 size) const;

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

    ui32 parseAsnLength();
    bool parsePDU(SeqASDU &seq);
    bool parseSequence(SeqASDU &seq);
    bool parseASDU(ASDU &asdu);

public:
    explicit IecParser();
    explicit IecParser(ui8 *data, const ui16 size);
    bool update(ui8 *data, const ui16 size);

    SeqASDU parse()
    {
        SeqASDU seq = { 0, nullptr };
        applyOffset(sizeof(ether_header));
        auto status = parsePDU(seq);
        assert(status);
        if (status)
        {
            status = parseSequence(seq);
            assert(status);
        }

        if (!status && seq.data)
        {
            delete[] seq.data;
            seq.data = nullptr;
        }

        return seq;
    }
};

}
