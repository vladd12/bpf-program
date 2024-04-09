#pragma once

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
    u16 smpCnt;                  ///< Sample count.
    u8 smpSynch;                 ///< Sample synchronisation.
    u32 confRev;                 ///< Config revision.
    DataUnit data[unitsPerASDU]; ///< Sequence of data.
};

/// \brief Class for parsing IEC-61850-9-2 SV LE frames.
class IecParser final
{
private:
    std::vector<ASDU> sequence;
    u16 internalFrameLength;

    inline bool verify(const u16 assumedLength) const noexcept
    {
        return internalFrameLength == assumedLength;
    }

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
        asduLength = asduLength - (sizeof(svId) + sizeof(svIdLength) + svIdLength);

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
        asduLength = asduLength - (sizeof(smpCntId) + sizeof(smpCntLen) + sizeof(smpCnt));

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
        asduLength = asduLength - (sizeof(confRevId) + sizeof(confRevLen) + sizeof(confRev));

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
        asduLength = asduLength - (sizeof(smpSyncId) + sizeof(smpSyncLen) + sizeof(smpSync));

        // reading sequence of data
        auto datasetId = buffer.readU8();
        --internalFrameLength;
        if (datasetId != stdDatasetId)
            return false;
        auto datasetLen = buffer.readU8();
        --internalFrameLength;
        constexpr auto assumeLen = sizeof(DataUnit) * unitsPerASDU;
        if (datasetLen != assumeLen)
            return false;
        memcpy(&asdu.data[0], buffer.get(), assumeLen);
        buffer.appendOffset(assumeLen);
        asduLength = asduLength - (sizeof(datasetId) + sizeof(datasetLen) + datasetLen);
        if (asduLength != 0)
            return false;
        return true;
    }

public:
    explicit IecParser() : internalFrameLength(0)
    {
    }

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
};

}
