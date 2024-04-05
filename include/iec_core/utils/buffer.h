#pragma once

#include <algorithm>
#include <iec_core/iec/byte_op.h>

namespace utils
{

struct StaticBuffer
{
public:
    using value_type = u8;
    using size_type = std::size_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;

private:
    static constexpr inline size_type size = 2048;

    value_type data[size];
    size_type offset = 0;

public:
    /// \brief Applying offset to the data pointer (to the frame buffer) and current the byte sequence size.
    /// \details Used when reading from the frame buffer occurs.
    bool apply(size_type offset_)
    {
        offset += offset_;
    }

    pointer get() noexcept
    {
        if (offset < size)
            return (&data[0] + offset);
        else
            return nullptr;
    }

    /// \brief Returns a byte (u8) from the frame buffer.
    u8 readU8()
    {
        return read<u8>();
    }

    /// \brief Returns a word (u16) from the frame buffer.
    u16 readU16()
    {
        auto _word = read<u16>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _word = bytes::byteswap(_word);
        return _word;
    }

    /// \brief Returns a double word (u32) from the frame buffer.
    u32 readU32()
    {
        auto _dword = read<u32>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _dword = bytes::byteswap(_dword);
        return _dword;
    }

    /// \brief Returns a quadro word (u64) from the frame buffer.
    u64 readU64()
    {
        auto _qword = read<u64>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _qword = bytes::byteswap(_qword);
        return _qword;
    }

    /// \brief Returns object with specified type from the frame buffer.
    template <typename T> //
    T read()
    {
        T value {};
        constexpr auto count = sizeof(T);
        if (size > (offset + count))
        {
            auto dstBegin = reinterpret_cast<u8 *>(&value);
            auto srcBegin = reinterpret_cast<const u8 *>(get());
            std::copy_n(srcBegin, count, dstBegin);
        }
        offset += count;
        return value;
    }
};

} // namespace utils
