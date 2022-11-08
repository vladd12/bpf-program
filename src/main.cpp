#include <BpfWrapper.h>
#include <cerrno>

// Linux
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int loadBpfProgrammSockPrepare(BpfWrapper *bpf, const std::string_view programPath, //
    const std::string_view functionName, const std::string_view deviceName)
{
    auto executeStatus = bpf->initByFile(programPath, deviceName);
    if (executeStatus.ok())
    {
        int fd_func = -1, sock_fd = -1;
        auto bpfObject = bpf->getBpfObject();
        executeStatus = bpfObject->load_func(functionName.data(), BPF_PROG_TYPE_SOCKET_FILTER, fd_func);
        if (executeStatus.ok())
        {
            // name of device may be eth0, eth1, etc (see ifconfig or ip a)...
            executeStatus = bpf->attachRawSocket(deviceName.data(), fd_func, sock_fd);
            if (executeStatus.ok())
                return sock_fd;
        }
    }
    printStatusMsg(executeStatus);
    return -1;
}

void test(int &sock)
{
    // Вывод данных о сокете
    auto fd = sock;
    auto optval = int(0);
    auto optlen = socklen_t(sizeof(optval));
    auto family = getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &optval, &optlen);
    std::cout << family << ' ' << optval << ' ' << AF_PACKET << '\n';
    auto type = getsockopt(fd, SOL_SOCKET, SO_TYPE, &optval, &optlen);
    std::cout << type << ' ' << optval << ' ' << SOCK_RAW << '\n';
    auto proto = getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &optval, &optlen);
    std::cout << proto << ' ' << optval << ' ' << htons(ETH_P_ALL) << "\n\n";
}

void printMacAddr(const std::uint8_t macAddr[])
{
    printf("0x%02X%02X%02X%02X%02X%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void test2(int &sock)
{
    constexpr std::size_t bufSize = 2048;
    auto buffer = new std::uint8_t[bufSize];
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
        }
    }
    delete[] buffer;
}

int main()
{
    using namespace std;
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    auto ifaceName = std::string("");
    std::cout << "Please, enter ethrnet interface name: ";
    std::cin >> ifaceName;
    auto sock = loadBpfProgrammSockPrepare(bpf.get(), "bpf/ethernet-parse.c", "iec61850_filter", ifaceName);
    if (sock >= 0)
    {
        // test(sock);
        test2(sock);
    }
    return 0;
}
