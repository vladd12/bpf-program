#include "bench.hpp"

#include <bpf_exec.h>
#include <iostream>
#include <sys/socket.h>

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

std::unique_ptr<BpfExec> bpfExecutor;
int socket;

void createRawSocket([[maybe_unused]] const benchmark::State &state)
{
    socket = BpfExec::getRawSocket(iface);
}

bool nativeFilter(std::uint8_t *data, std::size_t size)
{
    if (data && size)
        return true;
    else
        return false;
}

void createBpfSocket([[maybe_unused]] const benchmark::State &state)
{
    bpfExecutor = std::unique_ptr<BpfExec>(new BpfExec("bpf/ethernet-parse.c"));
    bpfExecutor->filterSourceCode(iface, "0x0cefaf3042cc", "ENS80pointMU01");
    auto status = bpfExecutor->run();
    if (status.ok())
    {
        int sock = -1;
        status = bpfExecutor->getDeviceSocket(sock, "iec61850_filter", iface);
        if (status.ok() && sock >= 0)
            socket = sock;
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
    for ([[maybe_unused]] auto _ : state)
    {
        while (counter != state.range(0))
        {
            [[maybe_unused]] auto receiveSize = recvfrom(socket, buffer, bufSize, 0, nullptr, nullptr);
            counter++;
        }
    }
    delete[] buffer;
}
BENCHMARK(BM_bpf)
    ->RangeMultiplier(2)
    ->Range(32, 4096)
    ->Iterations(100)
    ->Threads(1)
    ->Setup(createBpfSocket)
    ->Teardown(closeSocket);

void BM_native(benchmark::State &state)
{
    auto counter = 0;
    constexpr std::size_t bufSize = 2048;
    auto buffer = new std::uint8_t[bufSize];
    for ([[maybe_unused]] auto _ : state)
    {
        while (counter != state.range(0))
        {
            auto receiveSize = recvfrom(socket, buffer, bufSize, 0, nullptr, nullptr);
            if (nativeFilter(buffer, receiveSize))
                counter++;
        }
    }
    delete[] buffer;
}
BENCHMARK(BM_native)
    ->RangeMultiplier(2)
    ->Range(32, 4096)
    ->Iterations(100)
    ->Threads(1)
    ->Setup(createRawSocket)
    ->Teardown(closeSocket);

}
