#include <BpfWrapper.h>
#include <cerrno>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <sys/socket.h>

int loadBpfProgrammSockPrepare(BpfWrapper *bpf, const std::string_view programPath, //
    const std::string_view functionName, const std::string_view deviceName)
{
    auto executeStatus = bpf->initByFile(programPath.data());
    if (executeStatus.ok())
    {
        int fd_func = -1, sock_fd = -1;
        auto bpfObject = bpf->getBpfObject();
        executeStatus = bpfObject->load_func(functionName.data(), BPF_PROG_TYPE_SOCKET_FILTER, fd_func);
        if (executeStatus.ok())
        {
            // name of device may be eth0, eth1, etc (see ifconfig or ip -a)...
            executeStatus = bpf->attach_raw_socket(deviceName.data(), fd_func, sock_fd);
            if (executeStatus.ok())
                return sock_fd;
        }
    }
    printStatusMsg(executeStatus);
    return -1;
}

int main()
{
    using namespace std;
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    auto socket = loadBpfProgrammSockPrepare(bpf.get(), "bpf/ethernet-parse.c", "http_filter", "enp0s3");
    if (socket >= 0)
    {
        auto fd = socket;
        auto optval = int(0);
        auto optlen = socklen_t(sizeof(optval));
        auto family = getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &optval, &optlen);
        std::cout << family << ' ' << optval << ' ' << AF_PACKET << '\n';
        auto type = getsockopt(fd, SOL_SOCKET, SO_TYPE, &optval, &optlen);
        std::cout << type << ' ' << optval << ' ' << SOCK_RAW << '\n';

        uint16_t newProto = htons(ETH_P_ALL);
        if (setsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &newProto, sizeof(newProto)) < 0)
        {
            std::cout << errno << '\n';
            EINVAL;
        }

        auto proto = getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &optval, &optlen);
        std::cout << proto << ' ' << optval << ' ' << htons(ETH_P_ALL) << '\n';
    }

    return 0;
}
