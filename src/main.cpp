#include <BpfWrapper.h>
#include <cerrno>

// Linux
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

int loadBpfProgrammSockPrepare(BpfWrapper *bpf, const std::string_view programPath, //
    const std::string_view functionName, const std::string_view deviceName)
{
    auto executeStatus = bpf->initByFile(programPath);
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

constexpr std::size_t PACKET_BUF_SIZE = 2048;
struct packet_buf
{
    std::uint8_t data[PACKET_BUF_SIZE];
};

int main()
{
    using namespace std;
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    bpf->initByFile("bpf/eth-parse-var2.c");

    // auto ifaceName = std::string("");
    // std::cout << "Please, enter ethrnet interface name: ";
    // std::cin >> ifaceName;
    // auto sock = loadBpfProgrammSockPrepare(bpf.get(), "bpf/eth-parse-var2.c", "iec61850_filter", ifaceName);
    // if (sock >= 0)
    // {
    //    test2(sock);
    // }

    auto bpfObj = bpf->getBpfObject();
    const auto reader = [bpfObj]([[maybe_unused]] void *cpu, [[maybe_unused]] void *data, [[maybe_unused]] int size) -> void {
        auto packets = bpfObj->get_array_table<struct packet_buf>("xmits").get_table_offline();
        std::uint64_t *packPtr = nullptr;
        constexpr auto newSize = PACKET_BUF_SIZE / (sizeof(std::uint64_t) / sizeof(std::uint8_t));
        for (auto &&packet : packets)
        {
            packPtr = reinterpret_cast<std::uint64_t *>(&packet.data[0]);
            std::cout << "0x";
            for (auto i = std::size_t(0); i < newSize; i++)
            {
                printf("%016lX", *(packPtr + i));
            }
            std::cout << '\n';
        }
    };

    bpf->openPerfBuf("xmits", reader);

    return 0;
}
