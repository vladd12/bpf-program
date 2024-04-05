#pragma once

#include <iec_core/iec/byte_op.h>

/**
  Struct of IEC-61850-9-2 Sample Values Light Edition frame format
  Frame:
    - Ethernet header
    - PDU:
      - PDU Header
      - Sequence of ASDU:
        - ASDU 0:
          - Data unit 0:
            - Sample value
            - DetailQual
          ...
          - Data unit 7
        ...
        - ASDU N
**/

namespace iec
{

/// \brief Protocol data unit header of the SV LE frame.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
struct PDUHeader
{
    u16 appID;
    u16 length;
    u16 res1;
    u16 res2;
} __attribute__((__packed__));

/// \brief Detail quality bitset of a single sample value (current or voltage).
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
union DetailQual {
    u8 _data;
    struct __attribute__((__packed__))
    {
        u8 overflow : 1;
        u8 outOfRange : 1;
        u8 badReference : 1;
        u8 oscillatory : 1;
        u8 failure : 1;
        u8 oldData : 1;
        u8 inconsistent : 1;
        u8 inaccurate : 1;
    } data;
} __attribute__((__packed__));

/// \brief Data unit for transmitting the sample value and its quality.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
union DataUnit {
    u64 _data;
    struct __attribute__((__packed__))
    {
        u32 instMagI;
        u16 quality;
        union bitfield {
            u16 _data;
            struct __attribute__((__packed__))
            {
                u16 reserved : 2;
                u16 der : 1;
                u16 opB : 1;
                u16 test : 1;
                u16 src : 1;
                u16 dQual : 8;
                u16 valid : 2;
            } data;
        } bitset;
    } data;

    /// \brief Returns detail quality bitset of the sample value.
    auto getQuality() const
    {
        return DetailQual { u8(data.bitset.data.dQual) };
    }
} __attribute__((__packed__));

/// \brief Number of data units in the application sequence data unit.
/// \details The sequence of data units always contains 4 data units
/// for current values and 4 data units for voltage values. Each 4 data units
/// correspond to the values of the magnitude in phases A, B, C and neutral N.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
constexpr auto unitsPerASDU = 8;

/// \brief Application sequence data unit (ASDU).
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
struct ASDU
{
    u16 smpCnt;                  ///< Sample count.
    u8 smpSynch;                 ///< Sample synchronisation.
    u32 confRev;                 ///< Config revision.
    DataUnit data[unitsPerASDU]; ///< Sequence of data.
};

/// \brief Sequence of ASDU data structure.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
struct SeqASDU
{
    u8 count;
    ASDU *data;
};

/// \brief Class for parsing IEC-61850-9-2 SV LE frames.
class IecParser final
{
private:
    u8 *mData;
    u16 mSize;

    /// \brief Applying offset to the data pointer (to the frame buffer) and current the byte sequence size.
    /// \details Used when reading from the frame buffer occurs.
    bool applyOffset(u16 offset);

    /// \brief Verifying size of the byte sequence.
    bool verifySize(u16 size) const;

    /// \brief Returns a byte (u8) from the frame buffer.
    u8 readByte();
    /// \brief Returns a word (u16) from the frame buffer.
    u16 readWord();
    /// \brief Returns a double word (u32) from the frame buffer.
    u32 readDword();
    /// \brief Returns a quadro word (u64) from the frame buffer.
    u64 readQword();

    /// \brief Returns object with specified type from the frame buffer.
    template <typename T> T read()
    {
        T *val = reinterpret_cast<T *>(mData);
        applyOffset(sizeof(T));
        return *val;
    }

    /// \brief Parses the length of data in ASN format from the
    /// frame buffer and returns its integer representation.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    u32 parseAsnLength();

    /// \brief Parses the protocol data unit of frame.
    /// \param seq[in, out] - contains results of parsing.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    bool parsePDU(SeqASDU &seq);

    /// \brief Parses the sequence of ASDU from the frame buffer.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    bool parseSequence(SeqASDU &seq);

    /// \brief Parses the ASDU from the frame buffer.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    bool parseASDU(ASDU &asdu);

public:
    explicit IecParser();
    explicit IecParser(u8 *data, const u16 size);

    /// \brief Update internal pointer to the frame buffer with the frame data, and its size.
    bool update(u8 *data, const u16 size);

    /// \brief Returns the sequence of ASDU, parsed from the frame buffer.
    SeqASDU parse();
};

}
