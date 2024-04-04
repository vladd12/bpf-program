#include "bench.hpp"

#include <iec_core/engines/bpf_exec.h>
#include <iostream>
#include <net/ethernet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>

#define DEVICE 0
#define EMULATE 1
#define MACHINE EMULATE

namespace bm_bpf
{

#if MACHINE == EMULATE
constexpr auto iface = "enp0s8";
#else
constexpr auto iface = "eth0";
#endif

std::unique_ptr<BPFExecutor> bpfExecutor;
int socket;

void createRawSocket([[maybe_unused]] const benchmark::State &state)
{
    socket = BPFExecutor::getRawSocket(iface);
}

std::uint16_t byteswap(std::uint16_t val)
{
    std::uint16_t firstByte = (val >> 8) & 0x00ff;
    std::uint16_t secondByte = (val << 8) & 0xff00;
    return (firstByte | secondByte);
}

bool nativeFilter(std::uint8_t *data, std::size_t size)
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
        auto type = byteswap(ethernet->ether_type);
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

void createBpfSocket([[maybe_unused]] const benchmark::State &state)
{
    bpfExecutor = std::unique_ptr<BPFExecutor>(new BPFExecutor("bpf/ethernet-parse.c"));
    bpfExecutor->filterSourceCode(iface, "0x0cefaf3042cc", "ENS80pointMU01");
    auto status = bpfExecutor->load();
    if (status.ok())
    {
        int sock = -1;
        status = bpfExecutor->getDeviceSocket(sock, "iec61850_filter", iface);
        if (status.ok() && sock >= 0)
        {
            socket = sock;
            return;
        }
    }
    socket = -1;
}

void closeSocket([[maybe_unused]] const benchmark::State &state)
{
    if (socket >= 0)
        close(socket);
    if (bpfExecutor)
        bpfExecutor.reset();
}

void BM_bpf(benchmark::State &state)
{
    auto counter = 0;
    constexpr std::size_t bufSize = 2048;
    auto buffer = new std::uint8_t[bufSize];
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    for ([[maybe_unused]] auto _ : state)
    {
        while (counter != state.range(0))
        {
            state.PauseTiming();
            fd_set fd_in;
            FD_ZERO(&fd_in);
            FD_SET(socket, &fd_in);

            auto ret = select(FD_SETSIZE + 1, &fd_in, nullptr, nullptr, &tv);
            if (ret == -1)
            {
                // error
                fprintf(stderr, "Error checking socket aviability");
                break;
            }
            else if (ret == 0)
            {
                // timeout
                std::this_thread::yield();
            }
            else
            {
                // ok
                state.ResumeTiming();
                auto receiveSize = recvfrom(socket, buffer, bufSize, 0, nullptr, nullptr);
                if (receiveSize >= 0)
                    counter++;
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        counter = 0;
    }
    delete[] buffer;
}
BENCHMARK(BM_bpf)
    ->RangeMultiplier(2)
    ->Range(32, 1024)
    ->Iterations(100)
    ->Threads(1)
    ->Setup(createBpfSocket)
    ->Teardown(closeSocket);

void BM_native(benchmark::State &state)
{
    auto counter = 0;
    constexpr std::size_t bufSize = 2048;
    auto buffer = new std::uint8_t[bufSize];
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    for ([[maybe_unused]] auto _ : state)
    {
        while (counter != state.range(0))
        {
            state.PauseTiming();
            fd_set fd_in;
            FD_ZERO(&fd_in);
            FD_SET(socket, &fd_in);

            auto ret = select(FD_SETSIZE + 1, &fd_in, nullptr, nullptr, &tv);
            if (ret == -1)
            {
                // error
                fprintf(stderr, "Error checking socket aviability");
                break;
            }
            else if (ret == 0)
            {
                // timeout
                std::this_thread::yield();
            }
            else
            {
                // ok
                state.ResumeTiming();
                auto receiveSize = recvfrom(socket, buffer, bufSize, 0, nullptr, nullptr);
                if (nativeFilter(buffer, receiveSize))
                    counter++;
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        counter = 0;
    }
    delete[] buffer;
}
BENCHMARK(BM_native)
    ->RangeMultiplier(2)
    ->Range(32, 1024)
    ->Iterations(100)
    ->Threads(1)
    ->Setup(createRawSocket)
    ->Teardown(closeSocket);

}
