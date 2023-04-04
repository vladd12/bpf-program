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

}
