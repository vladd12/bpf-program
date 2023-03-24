#include <bpf_wrap.h>
#include <iostream>
#include <utils.h>

// Linux
#include <net/ethernet.h>
#include <sys/socket.h>

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;

constexpr word makeword(const byte &lhs, const byte &rhs)
{
    return (lhs << 8 | rhs);
}

void test(int &sock)
{
    constexpr std::size_t bufSize = 2048;
    constexpr std::size_t smpCnt80pOffset = 35;

    std::uint16_t max = 0;
    std::uint16_t min = UINT16_MAX;

    auto buffer = new std::uint8_t[bufSize];
    while (true)
    {
        auto rcStat = recvfrom(sock, buffer, bufSize, 0, nullptr, nullptr);
        if (rcStat >= 0)
        {
            auto iter = buffer;
            iter += sizeof(ether_header);
            iter += smpCnt80pOffset;
            auto smpCnt1 = reinterpret_cast<std::uint8_t *>(iter++);
            auto smpCnt2 = reinterpret_cast<std::uint8_t *>(iter++);
            auto smpCnt = makeword(*smpCnt1, *smpCnt2);
            auto id = reinterpret_cast<std::uint8_t *>(iter++);
            printf("Count: %04X %02X \n", smpCnt, *id);
            if (smpCnt > max)
                max = smpCnt;
            if (smpCnt < min)
                min = smpCnt;
            if (max > smpCnt && min < smpCnt)
                break;
        }
    }
    delete[] buffer;
    std::cout << "Min: " << min << "\nMax: " << max << "\n\n";
}

void inputData(std::string &iface, std::string &srcMac, std::string &svID)
{
    std::cout << "Enter the ethernet interface name: ";
    std::cin >> iface;
    std::cout << "Enter the source MAC address (format: 0xFFFFFFFFFFFF): ";
    std::cin >> srcMac;
    std::cout << "Enter the sample values ID: ";
    std::cin >> svID;
    // test
    std::cout << "Test input: " << iface << ' ' << srcMac << ' ' << svID << '\n';
}

int main()
{
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper("bpf/ethernet-parse.c"));
    auto ifaceName = std::string("enp0s8");          //
    auto srcMacAddr = std::string("0x0cefaf3042cc"); //
    auto svID = std::string("ENS80pointMU01");
    // inputData(ifaceName, srcMacAddr, svID);
    bpf->filterSourceCode(ifaceName, srcMacAddr, svID);

    auto status = bpf->run();
    if (status.ok())
    {
        int sock = -1;
        status = bpf->getDeviceSocket(sock, "iec61850_filter", ifaceName);
        if (status.ok() && sock >= 0)
        {
            test(sock);
        }
    }
    util::printStatusMessage(status);
    return 0;
}
