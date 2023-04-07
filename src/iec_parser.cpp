#include <iec_parser.h>

namespace iec
{

IecParser::IecParser() : mData(nullptr), mSize(0)
{
}

IecParser::IecParser(byte *data, const std::uint16_t size) : mData(data), mSize(size)
{
}

bool IecParser::update(byte *data, const std::uint16_t size)
{
    if (data != nullptr && size > 0)
    {
        mData = data;
        mSize = size;
        return true;
    }
    return false;
}

bool IecParser::applyOffset(std::uint16_t offset)
{
    if (mSize - offset >= 0)
    {
        mData += offset;
        mSize -= offset;
        return true;
    }
    return false;
}

bool IecParser::verifySize(std::uint16_t size)
{
    return size == mSize;
}

byte IecParser::readByte()
{
    auto _byte = read<byte>();
    return _byte;
}

word IecParser::readWord()
{
    auto _word = read<word>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _word = bytes::byteswap(_word);
    return _word;
}

dword IecParser::readDword()
{
    auto _dword = read<dword>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _dword = bytes::byteswap(_dword);
    return _dword;
}

qword IecParser::readQword()
{
    auto _qword = read<qword>();
    if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        _qword = bytes::byteswap(_qword);
    return _qword;
}

}
