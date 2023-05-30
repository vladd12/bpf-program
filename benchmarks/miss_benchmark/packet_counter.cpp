#include "packet_counter.h"

#include <bpf_exec.h>
#include <iostream>
#include <net/ethernet.h>

namespace detail
{

PacketCounter::PacketCounter(const std::string &iface, const std::string &mac, const std::string &svID)
    : buf { nullptr, util::BUFFER_SIZE, 0 }
{
    buf.data = new ui8[buf.allocSize];
    nativeSock.setHandle(BpfExec::getRawSocket(iface));
    std::unique_ptr<BpfExec> bpfExecutor(new BpfExec("bpf/ethernet-parse.c"));
    bpfExecutor->filterSourceCode(iface, mac, svID);
    auto status = bpfExecutor->run();
    if (status.ok())
    {
        int sock = -1;
        status = bpfExecutor->getDeviceSocket(sock, "iec61850_filter", iface);
        if (status.ok() && sock >= 0)
            bpfSock.setHandle(sock);
    }
    validator.setStrategy(net::Strategy::Statistics);
}

PacketCounter::~PacketCounter()
{
    delete[] buf.data;
    nativeSock.closeSock();
    bpfSock.closeSock();
}

bool PacketCounter::nativeFilter(std::uint8_t *data, std::size_t size)
{
    constexpr static auto svType1 = 0x8100;
    constexpr static auto svType2 = 0x88BA;
    constexpr static auto svIdSize = 35;
    constexpr static auto p80Offset = 18;
    constexpr static auto p256Offset = 22;

    if (data != nullptr && size > 0)
    {
        auto ethernet = reinterpret_cast<ether_header *>(data);
        data += sizeof(ether_header);
        size -= sizeof(ether_header);
        // auto type = byteswap(ethernet->ether_type);
        std::uint16_t firstByte = (ethernet->ether_type >> 8) & 0x00ff;
        std::uint16_t secondByte = (ethernet->ether_type << 8) & 0xff00;
        std::uint16_t type = firstByte | secondByte;
        if (type == svType1 || type == svType2)
        {
            char svId[svIdSize] = { 0 };
            // 80 point
            if (size >= 0x70 && size <= 0x100)
            {
                data += p80Offset;
                auto svIdLength = *data;
                data++;
                if (svIdLength < (svIdSize - 1))
                {
                    memcpy(&svId[0], data, svIdLength);
                    svId[svIdLength + 1] = '\0';
                }
                if (std::string(&svId[0]) == "ENS80pointMU01")
                    return true;
            }
            // 256 point
            else if (size >= 0x30b && size <= 0x500)
            {
                data += p256Offset;
                auto svIdLength = *data;
                data++;
                if (svIdLength < (svIdSize - 1))
                {
                    memcpy(&svId[0], data, svIdLength);
                    svId[svIdLength + 1] = '\0';
                }
                if (std::string(&svId[0]) == "ENS256MUnn01")
                    return true;
            }
        }
    }
    return false;
}

bool PacketCounter::bpfFilter(std::uint8_t *data, std::size_t size)
{
    if (data != nullptr && size > 0)
        return true;
    else
        return false;
}

void PacketCounter::printStatistic()
{
    std::cout << "Captured packets: " << validator.capturedCount << //
        "\nMissed packets: " << validator.missedCount << "\n\n";
}

void PacketCounter::readInTime(std::chrono::milliseconds time, TargetSocket target)
{
    using namespace std::chrono;
    nanoseconds detail_time = time;
    nanoseconds elapsed_time;
    bool status = true;
    auto start_time = high_resolution_clock::now();
    do
    {
        if (target == TargetSocket::Native)
        {
            nativeSock.nonBlockRead(buf);
            status = nativeFilter(buf.data, buf.readSize);
        }
        else if (target == TargetSocket::BPF)
        {
            bpfSock.nonBlockRead(buf);
            status = bpfFilter(buf.data, buf.readSize);
        }
        else
        {
            elapsed_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start_time);
            std::cout << "Unknown target\n";
            break;
        }

        if (status)
        {
            if (parser.update(buf.data, buf.readSize))
            {
                auto sequence = parser.parse();
                validator.update(sequence);

                // auto curr = sequence.data[0].smpCnt;
                // printf("Count: %04X\n", curr);

                if (sequence.data != nullptr)
                    delete[] sequence.data;
            }
        }

        elapsed_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start_time);
    } while (elapsed_time < detail_time);
    auto elapsed_time_ms = elapsed_time.count() / 1000000;
    printStatistic();
    std::cout << "Time: " << elapsed_time_ms << "ms.\n";
    validator.reset();
}

// void PacketCounter::readPackets(std::uint64_t numOfPackets, TargetSocket target)
//{
//    bool status = true;
//    while (validator.capturedCount != numOfPackets)
//    {
//        if (target == TargetSocket::Native)
//        {
//            nativeSock.nonBlockRead(buf);
//            status = nativeFilter(buf.data, buf.readSize);
//        }
//        else if (target == TargetSocket::BPF)
//        {
//            bpfSock.nonBlockRead(buf);
//            status = bpfFilter(buf.data, buf.readSize);
//        }
//        else
//        {
//            std::cout << "Unknown target\n";
//            break;
//        }
//        if (status)
//        {
//            if (parser.update(buf.data, buf.readSize))
//            {
//                auto sequence = parser.parse();
//                validator.update(sequence);
//                // auto curr = sequence.data[0].smpCnt;
//                // printf("Count: %04X\n", curr);
//                if (sequence.data != nullptr)
//                    delete[] sequence.data;
//            }
//        }
//    }
//    printStatistic();
//    validator.reset();
//}

void PacketCounter::readPacketsNative(std::uint64_t numOfPackets)
{
    while (validator.capturedCount != numOfPackets)
    {
        nativeSock.nonBlockRead(buf);
        if (nativeFilter(buf.data, buf.readSize))
        {
            if (parser.update(buf.data, buf.readSize))
            {
                auto sequence = parser.parse();
                // auto curr = sequence.data[0].smpCnt;
                // printf("Count: %04X\n", curr);
                validator.update(sequence);
                if (sequence.data != nullptr)
                    delete[] sequence.data;
            }
        }
    }
    printStatistic();
    validator.reset();
}

void PacketCounter::readPacketsBpf(std::uint64_t numOfPackets)
{
    while (validator.capturedCount != numOfPackets)
    {
        bpfSock.nonBlockRead(buf);
        if (bpfFilter(buf.data, buf.readSize))
        {
            if (parser.update(buf.data, buf.readSize))
            {
                auto sequence = parser.parse();
                // auto curr = sequence.data[0].smpCnt;
                // printf("Count: %04X\n", curr);
                validator.update(sequence);
                if (sequence.data != nullptr)
                    delete[] sequence.data;
            }
        }
    }
    printStatistic();
    validator.reset();
}

}
