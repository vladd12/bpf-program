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
    if (mSize - offset > 0)
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
    auto _byte = *mData;
    applyOffset(sizeof(byte));
    return _byte;
}

word IecParser::readWord()
{
    using namespace bytes;
    auto _word = makeword(*mData, *(mData + 1));
    applyOffset(sizeof(word));
    return _word;
}

dword IecParser::readDword()
{
    using namespace bytes;
    auto _dword = makedword(makeword(*mData, *(mData + 1)), makeword(*(mData + 2), *(mData + 3)));
    applyOffset(sizeof(dword));
    return _dword;
}

qword IecParser::readQword()
{
    using namespace bytes;
    auto _dword1 = makedword(makeword(*mData, *(mData + 1)), makeword(*(mData + 2), *(mData + 3)));
    auto _dword2 = makedword(makeword(*(mData + 4), *(mData + 5)), makeword(*(mData + 6), *(mData + 7)));
    applyOffset(sizeof(qword));
    return makeqword(_dword1, _dword2);
}

}
