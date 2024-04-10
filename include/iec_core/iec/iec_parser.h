#pragma once

#include <array>
#include <iec_core/utils/buffer.h>
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
        i32 instMagI;
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
    u16 smpCnt;                              ///< Sample count.
    u8 smpSynch;                             ///< Sample synchronisation.
    u32 confRev;                             ///< Config revision.
    std::array<DataUnit, unitsPerASDU> data; ///< Sequence of data.
};

/// \brief Real representation for ASDU's data unit.
struct Point
{
    std::array<f32, unitsPerASDU> values;
};

/// \brief Class for parsing IEC-61850-9-2 SV LE frames.
class IecParser final
{
private:
    std::vector<ASDU> sequence;
    u16 internalFrameLength;

    /// \brief Verifying size of the byte sequence.
    inline bool verify(const u16 assumedLength) const noexcept
    {
        return internalFrameLength == assumedLength;
    }

    /// \brief Parses the length of data in ASN format from the
    /// frame buffer and returns its integer representation.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    template <std::size_t size> //
    inline u32 parseAsnLength(utils::StaticBuffer<size> &buffer)
    {
        auto firstByte = buffer.readU8();
        --internalFrameLength;
        u32 length = 0;
        if (firstByte & 0x80)
        {
            auto asnLength = firstByte & 0x7f;
            if (asnLength > 4)
                return 0;
            while (asnLength > 0)
            {
                length += buffer.readU8();
                --internalFrameLength;
                asnLength--;
                if (asnLength > 0)
                    length = length << 8;
            }
            return length;
        }
        else
            return firstByte;
    }

    /// \brief Parses the protocol data unit of frame.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    template <std::size_t size> //
    inline bool parsePDU(utils::StaticBuffer<size> &buffer)
    {
        constexpr u8 stdPduId = 0x60;
        constexpr u8 stdNoAsduId = 0x80;
        PDUHeader head { buffer.readU16(), buffer.readU16(), buffer.readU16(), buffer.readU16() };
        if (head.length <= 0)
            return false;
        internalFrameLength = head.length - sizeof(head);

        // reading savPDU
        auto pduId = buffer.readU8();
        --internalFrameLength;
        if (pduId != stdPduId)
            return false;
        auto length = parseAsnLength(buffer);
        if (!verify(length))
            return false;

        // reading noASDU
        auto noAsduId = buffer.readU8();
        --internalFrameLength;
        if (noAsduId != stdNoAsduId)
            return false;
        length = buffer.readU8();
        --internalFrameLength;
        if (length != 1)
            return false;
        auto seqCount = buffer.readU8();
        --internalFrameLength;
        if (seqCount > 16 || seqCount < 1)
            return false;
        return parseSequence(seqCount, buffer);
    }

    /// \brief Parses the sequence of ASDU from the frame buffer.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    template <std::size_t size> //
    inline bool parseSequence(const u8 seqCount, utils::StaticBuffer<size> &buffer)
    {
        // reading seqASDU
        constexpr u8 stdSeqAsduId = 0xa2;
        auto seqAsduId = buffer.readU8();
        --internalFrameLength;
        if (seqAsduId != stdSeqAsduId)
            return false;
        auto length = parseAsnLength(buffer);
        if (!verify(length))
            return false;

        // reading ASDU
        for (auto i = 0; i < seqCount; ++i)
        {
            if (!parseASDU(buffer))
                return false;
        }
        if (!verify(0))
            return false;
        return true;
    }

    /// \brief Parses the ASDU from the frame buffer.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    template <std::size_t size> //
    inline bool parseASDU(utils::StaticBuffer<size> &buffer)
    {
        constexpr u8 stdAsduId = 0x30;
        constexpr u8 stdSvId = 0x80;
        constexpr u8 stdSmpCntId = 0x82;
        constexpr u8 stdConfRevId = 0x83;
        constexpr u8 stdSmpSyncId = 0x85;
        constexpr u8 stdDatasetId = 0x87;
        ASDU asdu;

        // reading ASDU header
        auto asduId = buffer.readU8();
        --internalFrameLength;
        if (asduId != stdAsduId)
            return false;
        auto asduLength = buffer.readU8();
        --internalFrameLength;
        if (asduLength < 91 || asduLength > 115)
            return false;

        // reading svID
        auto svId = buffer.readU8();
        --internalFrameLength;
        if (svId != stdSvId)
            return false;
        auto svIdLength = buffer.readU8();
        --internalFrameLength;
        if (svIdLength < 10 || svIdLength > 34)
            return false;
        buffer.appendOffset(svIdLength);
        internalFrameLength -= svIdLength;
        asduLength -= (sizeof(svId) + sizeof(svIdLength) + svIdLength);

        // reading smpCnt
        auto smpCntId = buffer.readU8();
        --internalFrameLength;
        if (smpCntId != stdSmpCntId)
            return false;
        auto smpCntLen = buffer.readU8();
        --internalFrameLength;
        if (smpCntLen != 2)
            return false;
        auto smpCnt = buffer.readU16();
        internalFrameLength -= sizeof(smpCnt);
        asdu.smpCnt = smpCnt;
        asduLength -= (sizeof(smpCntId) + sizeof(smpCntLen) + sizeof(smpCnt));

        // reading confRev
        auto confRevId = buffer.readU8();
        --internalFrameLength;
        if (confRevId != stdConfRevId)
            return false;
        auto confRevLen = buffer.readU8();
        --internalFrameLength;
        if (confRevLen != 4)
            return false;
        auto confRev = buffer.readU32();
        internalFrameLength -= sizeof(confRev);
        asdu.confRev = confRev;
        asduLength -= (sizeof(confRevId) + sizeof(confRevLen) + sizeof(confRev));

        // reading smpSynch
        auto smpSyncId = buffer.readU8();
        --internalFrameLength;
        if (smpSyncId != stdSmpSyncId)
            return false;
        auto smpSyncLen = buffer.readU8();
        --internalFrameLength;
        if (smpSyncLen != 1)
            return false;
        auto smpSync = buffer.readU8();
        --internalFrameLength;
        asdu.smpSynch = smpSync;
        asduLength -= (sizeof(smpSyncId) + sizeof(smpSyncLen) + sizeof(smpSync));

        // reading sequence of data
        auto datasetId = buffer.readU8();
        --internalFrameLength;
        if (datasetId != stdDatasetId)
            return false;
        auto datasetLen = buffer.readU8();
        --internalFrameLength;
        asduLength -= (sizeof(datasetId) + sizeof(datasetLen));
        constexpr auto assumeLen = sizeof(DataUnit) * unitsPerASDU;
        if (datasetLen != assumeLen)
            return false;

        // reading data units of ASDU
        for (auto i = 0; i < unitsPerASDU; ++i)
            parseDataUnit(asdu.data[i], buffer);
        sequence.push_back(std::move(asdu));
        asduLength -= datasetLen;
        if (asduLength != 0)
            return false;
        return true;
    }

    /// \brief Parses the ASDU's data unit from the frame buffer.
    /// \see IEC-61850-9-2 Sample Values Light Edition frame format.
    template <std::size_t size> //
    inline void parseDataUnit(DataUnit &unit, utils::StaticBuffer<size> &buffer)
    {
        unit.data.instMagI = static_cast<i32>(buffer.readU32());
        internalFrameLength -= sizeof(unit.data.instMagI);
        unit.data.quality = buffer.readU16();
        internalFrameLength -= sizeof(unit.data.quality);
        unit.data.bitset._data = buffer.readU16();
        internalFrameLength -= sizeof(unit.data.bitset._data);
    }

public:
    explicit IecParser() : internalFrameLength(0)
    {
    }

    /// \brief Returns the sequence of ASDU, parsed from the frame buffer.
    template <std::size_t size> //
    inline std::vector<ASDU> parse(utils::StaticBuffer<size> &buffer)
    {
        sequence.reserve(128);
        while (buffer.getOffset() < buffer.getWritten())
        {
            buffer.appendOffset(sizeof(ether_header));
            parsePDU(buffer);
        }
        return std::vector<ASDU> { std::move(sequence) };
    }

    /// \brief Returns the real representation of the ASDU sequence, parsed from the frame buffer.
    inline std::vector<Point> convert(const std::vector<ASDU> &sequence_)
    {
        constexpr f32 ampScaleFactor = 0.001f;
        constexpr f32 volScaleFactor = 0.01f;
        Point point {};
        std::vector<Point> points;
        points.reserve(sequence_.size());
        for (const auto &asdu : sequence_)
        {
            for (auto i = 0; i < unitsPerASDU; ++i)
            {
                const auto scaleFactor = ((i < unitsPerASDU / 2) ? ampScaleFactor : volScaleFactor);
                point.values[i] = static_cast<f32>(asdu.data[i].data.instMagI) * scaleFactor;
            }
            points.push_back(std::move(point));
        }
        return points;
    }
};

}
