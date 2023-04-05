#pragma once

#include <byte_op.h>
#include <net/ethernet.h>

namespace iec
{
struct PDUHeader
{
    std::uint16_t appID;
    std::uint16_t length;
    std::uint16_t res1;
    std::uint16_t res2;
} __attribute__((__packed__));

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

    bool applyOffset(std::uint16_t offset);
    bool verifySize(std::uint16_t size);

    byte readByte();
    word readWord();
    dword readDword();
    qword readQword();

    template <typename T> T read()
    {
        T *ptr = reinterpret_cast<T *>(mData);
        applyOffset(sizeof(T));
        return *ptr;
    }

    std::uint32_t parseAsnLength()
    {
        auto firstByte = readByte();
        std::uint32_t length = 0;

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

public:
    explicit IecParser();
    explicit IecParser(byte *data, const std::uint16_t size);
    bool update(byte *data, const std::uint16_t size);

    SeqASDU parse()
    {
        SeqASDU seq;
        applyOffset(sizeof(ether_header));
        [[maybe_unused]] auto head = read<PDUHeader>();
        return seq;
    }
};

}
