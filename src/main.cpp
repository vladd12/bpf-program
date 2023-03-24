#include <bpf_wrap.h>
#include <iostream>
#include <utils.h>

// Linux
#include <net/ethernet.h>
#include <sys/socket.h>

void test(int &sock)
{
    constexpr std::size_t bufSize = 2048;
    auto buffer = new std::uint8_t[bufSize];
    std::uint64_t count = 0;
    while (true)
    {
        auto rcStat = recvfrom(sock, buffer, bufSize, 0, nullptr, nullptr);
        if (rcStat >= 0)
        {
            auto iter = buffer;
            auto ethernetHeader = reinterpret_cast<ether_header *>(iter);
            std::cout << "Dst MAC address: ";
            util::printMacAddress(ethernetHeader->ether_dhost);
            std::cout << "Src MAC address: ";
            util::printMacAddress(ethernetHeader->ether_shost);
            iter += sizeof(ether_header);
            printf("Count: %010lu \n\n", count);
            count = count + 1;
        }
    }
    delete[] buffer;
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
