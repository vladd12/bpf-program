#include <bpf_exec.h>
#include <chrono>
#include <iostream>
#include <net/sock.h>
#include <net/validator.h>
#include <thread>
#include <utility>

std::pair<net::Socket, net::Socket> init()
{
    constexpr auto iface = "enp0s8";
    constexpr auto mac = "0x0cefaf3042cc";
    constexpr auto svID = "ENS80pointMU01";
    net::Socket nativeSock(BpfExec::getRawSocket(iface));
    std::unique_ptr<BpfExec> bpfExecutor(new BpfExec("bpf/ethernet-parse.c"));
    bpfExecutor->filterSourceCode(iface, mac, svID);
    auto status = bpfExecutor->run();
    if (status.ok())
    {
        int sock = -1;
        status = bpfExecutor->getDeviceSocket(sock, "iec61850_filter", iface);
        if (status.ok() && sock >= 0)
        {
            net::Socket bpfSock(sock);
            return { nativeSock, bpfSock };
        }
    }
    return { nativeSock, net::Socket(-1) };
}

// TODO
void nativeFilter()
{
    ;
}

void readForTime(std::chrono::milliseconds time, net::Socket sock)
{
    using namespace std::chrono;
    nanoseconds detail_time = time;
    nanoseconds elapsed_time;
    net::Validator validator;
    auto start_time = high_resolution_clock::now();
    do
    {
        elapsed_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start_time);
        std::cout << elapsed_time.count() << " ns.\n";
    } while (elapsed_time < detail_time);
}

int main()
{
    using namespace std::chrono_literals;
    auto sockets = init();
    auto nativeSock = sockets.first, bpfSock = sockets.second;
    readForTime(10ms, nativeSock);

    nativeSock.closeSock();
    bpfSock.closeSock();
    return 0;
}
