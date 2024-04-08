#pragma once

#include <algorithm>
#include <iec_core/iec/byte_op.h>

namespace utils
{

/// \brief Class for using static memory buffer.
template <std::size_t size> //
struct StaticBuffer final
{
    friend struct StaticBufferRef;

public:
    using value_type = u8;
    using size_type = std::size_t;
    using pointer = value_type *;

private:
    value_type data[size];
    size_type offset = 0;
    size_type written = 0;

public:
    inline explicit StaticBuffer() noexcept = default;

    /// \brief Returns pointer to the data with using current offset.
    /// \details If the current offset more than size of
    /// the data array, function returns nullptr.
    inline pointer get() noexcept
    {
        if (offset < size)
            return (&data[0] + offset);
        else
            return nullptr;
    }

    /// \brief Returns pointer to the data with using written offset.
    /// \details If the written offset more than size of
    /// the data array, function returns nullptr.
    inline pointer getFree() noexcept
    {
        if (written < size)
            return (&data[0] + written);
        else
            return nullptr;
    }

    /// \brief Append to the current offset some length.
    inline void appendOffset(size_type length) noexcept
    {
        offset += length;
    }

    /// \brief Append to the written offset some length.
    inline void appendWritten(size_type length) noexcept
    {
        written += length;
    }

    /// \brief Returns free size.
    inline size_type getSize() const noexcept
    {
        return size - written;
    }

    /// \brief Reset some counters for buffer.
    inline void reset() noexcept
    {
        offset = 0;
        written = 0;
    }

    /// \brief Returns a byte (u8) from the frame buffer.
    inline u8 readU8() noexcept
    {
        return read<u8>();
    }

    /// \brief Returns a word (u16) from the frame buffer.
    inline u16 readU16() noexcept
    {
        auto _word = read<u16>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _word = bytes::byteswap(_word);
        return _word;
    }

    /// \brief Returns a double word (u32) from the frame buffer.
    inline u32 readU32() noexcept
    {
        auto _dword = read<u32>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _dword = bytes::byteswap(_dword);
        return _dword;
    }

    /// \brief Returns a quadro word (u64) from the frame buffer.
    inline u64 readU64() noexcept
    {
        auto _qword = read<u64>();
        if constexpr (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
            _qword = bytes::byteswap(_qword);
        return _qword;
    }

    /// \brief Returns object with specified type from the frame buffer.
    template <typename T> //
    inline T read() noexcept
    {
        T value {};
        constexpr auto count = sizeof(T);
        if (size > (offset + count))
        {
            auto dstBegin = reinterpret_cast<u8 *>(&value);
            auto srcBegin = reinterpret_cast<const u8 *>(get());
            std::copy_n(srcBegin, count, dstBegin);
        }
        appendOffset(count);
        return value;
    }
};

/// TODO?
// struct StaticBufferRef
//{
// public:
//    using value_type = u8;
//    using size_type = std::size_t;
//    using pointer = value_type *;
// private:
//    pointer data;
//    size_type size, offset;
// public:
//    template <std::size_t size> //
//    explicit StaticBufferRef(StaticBuffer<size> &buffer, size_type size_) noexcept : data(buffer.get()), size(size_), offset(0)
//    {
//    }
//};

} // namespace utils
