#include <BpfWrapper.h>

// Linux
#include <net/ethernet.h>
#include <sys/socket.h>

int loadBpfProgrammSockPrepare(BpfWrapper *bpf, const std::string &programPath, //
    const std::string &funcName, const std::string &ifaceName, const std::string &srcMac, const std::string &svID)
{
    bpf->initByFile(programPath);
    auto status = bpf->filterProgText(ifaceName, srcMac, svID);
    if (status.ok())
    {
        int fd_func = -1, sock_fd = -1;
        auto bpfObject = bpf->getBpfObject();
        status = bpfObject->load_func(funcName, BPF_PROG_TYPE_SOCKET_FILTER, fd_func);
        if (status.ok())
        {
            // name of device may be eth0, eth1, etc (see ifconfig or ip a)...
            status = bpf->attachRawSocket(ifaceName, fd_func, sock_fd);
            if (status.ok())
                return sock_fd;
        }
    }
    printStatusMsg(status);
    return -1;
}

void printMacAddr(const std::uint8_t macAddr[])
{
    printf("0x%02X%02X%02X%02X%02X%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void test2(int &sock)
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
            printMacAddr(ethernetHeader->ether_dhost);
            std::cout << "Src MAC address: ";
            printMacAddr(ethernetHeader->ether_shost);
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
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    auto ifaceName = std::string("enp0s8");          //
    auto srcMacAddr = std::string("0x0cefaf3042cc"); //
    auto svID = std::string("ENS80pointMU01");
    inputData(ifaceName, srcMacAddr, svID);

    auto sock = loadBpfProgrammSockPrepare(bpf.get(), "bpf/ethernet-parse.c", "iec61850_filter", ifaceName, srcMacAddr, svID);
    if (sock >= 0)
    {
        // test(sock);
        test2(sock);
    }
    return 0;
}
