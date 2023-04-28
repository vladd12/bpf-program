#pragma once

#include <byte_op.h>
#include <cassert>
#include <cstring>
#include <net/ethernet.h>

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
    ui16 appID;
    ui16 length;
    ui16 res1;
    ui16 res2;
} __attribute__((__packed__));

/// \brief Detail quality bitset of a single sample value (current or voltage).
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
union DetailQual {
    ui8 _data;
    struct __attribute__((__packed__))
    {
        ui8 overflow : 1;
        ui8 outOfRange : 1;
        ui8 badReference : 1;
        ui8 oscillatory : 1;
        ui8 failure : 1;
        ui8 oldData : 1;
        ui8 inconsistent : 1;
        ui8 inaccurate : 1;
    } data;
};

/// \brief Data unit for transmitting the sample value and its quality.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
union DataUnit {
    ui64 _data;
    struct __attribute__((__packed__))
    {
        ui32 instMagI;
        ui16 quality;
        union bitfield {
            ui16 _data;
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
        } bitset;
    } data;

    /// \brief Returns detail quality bitset of the sample value.
    auto getQuality() const
    {
        return DetailQual { ui8(data.bitset.data.dQual) };
    }
};

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
    ui16 smpCnt;                 ///< Sample count.
    ui32 confRev;                ///< Config revision.
    ui8 smpSynch;                ///< Sample synchronisation.
    DataUnit data[unitsPerASDU]; ///< Sequence of data.
};

/// \brief Sequence of ASDU data structure.
/// \see IEC-61850-9-2 Sample Values Light Edition frame format.
struct SeqASDU
{
    ui8 count;
    ASDU *data;
};

/// \brief Class for parsing IEC-61850-9-2 SV LE frames.
class IecParser final
{
private:
    ui8 *mData;
    ui16 mSize;

    /// \brief Applying offset to the data pointer (to the frame buffer) and current the byte sequence size.
    /// \details Used when reading from the frame buffer occurs.
    bool applyOffset(ui16 offset);

    /// \brief Verifying size of the byte sequence.
    bool verifySize(ui16 size) const;

    /// \brief Returns a byte (ui8) from the frame buffer.
    ui8 readByte();
    /// \brief Returns a word (ui16) from the frame buffer.
    ui16 readWord();
    /// \brief Returns a double word (ui32) from the frame buffer.
    ui32 readDword();
    /// \brief Returns a quadro word (ui64) from the frame buffer.
    ui64 readQword();

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
    ui32 parseAsnLength();

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
    explicit IecParser(ui8 *data, const ui16 size);

    /// \brief Update internal pointer to the frame buffer with the frame data, and its size.
    bool update(ui8 *data, const ui16 size);

    /// \brief Returns the sequence of ASDU, parsed from the frame buffer.
    SeqASDU parse();
};

}
