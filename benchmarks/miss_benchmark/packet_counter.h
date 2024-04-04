#pragma once

#include <bpf_core/iec/validator.h>
#include <bpf_core/utils/socket.h>
#include <chrono>
#include <string>

namespace detail
{
using iec::IecParser;
using iec::Validator;
using utils::Buffer;
using utils::Socket;

enum class TargetSocket : u8
{
    Native = 0,
    BPF
};

class PacketCounter
{
private:
    Socket nativeSock;
    Socket bpfSock;
    Validator validator;
    Buffer buf;
    IecParser parser;

    bool nativeFilter(std::uint8_t *data, std::size_t size);
    bool bpfFilter(std::uint8_t *data, std::size_t size);
    void printStatistic();

public:
    explicit PacketCounter(const std::string &iface, const std::string &mac, const std::string &svID);
    ~PacketCounter();
    void readInTime(std::chrono::milliseconds time, TargetSocket target);
    void readPacketsNative(std::uint64_t numOfPackets);
    void readPacketsBpf(std::uint64_t numOfPackets);
};

}
