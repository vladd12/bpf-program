#pragma once

#include <chrono>
#include <net/sock.h>
#include <net/validator.h>
#include <string>

namespace detail
{
using iec::IecParser;
using net::Socket;
using net::Validator;
using util::Buffer;

enum class TargetSocket : ui8
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
    void printStatistic(std::chrono::milliseconds time);

public:
    explicit PacketCounter(const std::string &iface, const std::string &mac, const std::string &svID);
    ~PacketCounter();
    void readInTime(std::chrono::milliseconds time, TargetSocket target);
};

}
