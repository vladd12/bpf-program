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

void printFrame(const std::uint32_t *frame, const std::size_t &size) {
    std::cerr << "0x";
    for (std::size_t i = 0, j = 0; j < size; i++, j = j + sizeof(std::uint32_t)) {
        printf("%08X", frame[i]);
    }
    std::cerr << '\n';
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

constexpr std::size_t TASK_COMM_LEN = 16;
struct xmit_event {
    std::uint64_t ts;
    std::uint32_t pid;
    std::uint32_t tgid;
    std::uint32_t len;
    std::uint32_t datalen;
    std::uint32_t packet_buf_ptr;
    char comm[TASK_COMM_LEN];
    std::uint64_t head;
    std::uint64_t data;
    std::uint64_t tail;
    std::uint64_t end;
};

constexpr std::size_t PACKET_BUF_SIZE = 2048;
struct packet_buf
{
    std::uint8_t data[PACKET_BUF_SIZE];
};

int main()
{
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    auto bpfObj = bpf->getBpfObject();

    auto reader = [](void *cb_cookie, void *data, int size) -> void {
        std::cout << "Size: " << size << '\n';
        if (size > 0)
        {
            auto frame = reinterpret_cast<xmit_event *>(data);

        }
    };

    auto status = bpf->initByFile("bpf/eth-parse-var2.c");
    if (status.ok())
    {
        int prog_fd = 0;

        status = bpfObj->load_func("kprobe____dev_queue_xmit", BPF_PROG_TYPE_KPROBE, prog_fd);
        if (status.ok())
        {
            // bpfObj->attach_kprobe("__dev_queue_xmit", "kprobe____dev_queue_xmit");

            auto ret = bpf_attach_kprobe(prog_fd, BPF_PROBE_ENTRY, "kprobe____dev_queue_xmit", "__dev_queue_xmit", 0, 0);
            if (ret >= 0)
            {
                status = bpfObj->open_perf_buffer("xmits", reader);
                if (status.ok())
                {
                    auto perf_buff = bpfObj->get_perf_buffer("xmits");
                    if (perf_buff != nullptr) {
                    while (true)
                    {
                        auto pollStat = perf_buff->poll(-1);
                        if (pollStat < 0)
                        {
                            std::cout << "Polling error, data no exist!\n\n";
                        }
                        else if (pollStat == 0)
                        {
                            std::cout << "Buffer is empty!\n\n";
                        }
                        else
                        {
                            ;
                            ;
                        }
                    }
                    }
                    else status = ebpf::StatusTuple(-1, "Unable to getting perf buffer!");
                }
            }
            else
                status = ebpf::StatusTuple(-1, "Unable to attach kprobe!");
        }
    }
    printStatusMsg(status);

    return 0;
}
