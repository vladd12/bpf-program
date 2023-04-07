#include <iec_parser.h>

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

bool IecParser::verifySize(ui16 size)
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

}
