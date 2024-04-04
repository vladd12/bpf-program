#include "bpf_core/iec_parser.h"

#include <cassert>
#include <cstring>
#include <net/ethernet.h>

namespace iec
{

IecParser::IecParser() : mData(nullptr), mSize(0)
{
}

IecParser::IecParser(ui8 *data, const ui16 size) : mData(data), mSize(size)
{
}

bool IecParser::update(ui8 *data, const ui16 size)
{
    if (data != nullptr && size > 0)
    {
        mData = data;
        mSize = size;
        return true;
    }
    return false;
}

bool IecParser::applyOffset(ui16 offset)
{
    if (mSize - offset >= 0)
    {
        mData += offset;
        mSize -= offset;
        return true;
    }
    return false;
}

bool IecParser::verifySize(ui16 size) const
{
    return size == mSize;
}

ui8 IecParser::readByte()
{
    auto _byte = read<ui8>();
    return _byte;
}

ui16 IecParser::readWord()
{
    auto _word = read<ui16>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _word = bytes::byteswap(_word);
    return _word;
}

ui32 IecParser::readDword()
{
    auto _dword = read<ui32>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _dword = bytes::byteswap(_dword);
    return _dword;
}

ui64 IecParser::readQword()
{
    auto _qword = read<ui64>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _qword = bytes::byteswap(_qword);
    return _qword;
}

ui32 IecParser::parseAsnLength()
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

bool IecParser::parsePDU(SeqASDU &seq)
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

bool IecParser::parseSequence(SeqASDU &seq)
{
    // reading seqASDU
    auto seqAsduId = readByte();
    if (seqAsduId != 0xa2)
        return false;
    ui32 length = parseAsnLength();
    if (!verifySize(length))
        return false;

    seq.data = new ASDU[seq.count];
    for (auto i = 0; i < seq.count; i++)
    {
        // reading ASDU
        if (!parseASDU(seq.data[i]))
            return false;
    }
    if (!verifySize(0))
        return false;

    return true;
}

bool IecParser::parseASDU(ASDU &asdu)
{
    // reading ASDU header
    auto asduId = readByte();
    if (asduId != 0x30)
        return false;
    auto asduLength = readByte();
    if (asduLength < 91 || asduLength > 115)
        return false;

    // reading svID
    auto svId = readByte();
    if (svId != 0x80)
        return false;
    auto svIdLength = readByte();
    if (svIdLength < 10 || svIdLength > 34)
        return false;
    applyOffset(svIdLength);
    asduLength = asduLength - (sizeof(svId) + sizeof(svIdLength) + svIdLength);

    // reading smpCnt
    auto smpCntId = readByte();
    if (smpCntId != 0x82)
        return false;
    auto smpCntLen = readByte();
    if (smpCntLen != 2)
        return false;
    auto smpCnt = readWord();
    asdu.smpCnt = smpCnt;
    asduLength = asduLength - (sizeof(smpCntId) + sizeof(smpCntLen) + sizeof(smpCnt));

    // reading confRev
    auto confRevId = readByte();
    if (confRevId != 0x83)
        return false;
    auto confRevLen = readByte();
    if (confRevLen != 4)
        return false;
    auto confRev = readDword();
    asdu.confRev = confRev;
    asduLength = asduLength - (sizeof(confRevId) + sizeof(confRevLen) + sizeof(confRev));

    // reading smpSynch
    auto smpSyncId = readByte();
    if (smpSyncId != 0x85)
        return false;
    auto smpSyncLen = readByte();
    if (smpSyncLen != 1)
        return false;
    auto smpSync = readByte();
    asdu.smpSynch = smpSync;
    asduLength = asduLength - (sizeof(smpSyncId) + sizeof(smpSyncLen) + sizeof(smpSync));

    // reading sequence of data
    auto datasetId = readByte();
    if (datasetId != 0x87)
        return false;
    auto datasetLen = readByte();
    constexpr auto assumeLen = sizeof(DataUnit) * unitsPerASDU;
    if (datasetLen != assumeLen)
        return false;
    memcpy(&asdu.data[0], mData, assumeLen);
    applyOffset(assumeLen);
    asduLength = asduLength - (sizeof(datasetId) + sizeof(datasetLen) + datasetLen);
    if (asduLength != 0)
        return false;
    return true;
}

SeqASDU IecParser::parse()
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

}
