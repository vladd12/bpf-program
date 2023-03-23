#include <bpf_wrap.h>
#include <cstdio>
#include <fstream>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <utils.h>

BpfWrapper::BpfWrapper(const std::string &programPath) : bpfPtr(new ebpf::BPF), bpfSrc(util::read_file(programPath))
{
}

int BpfWrapper::get_raw_socket(const std::string &ifaceName)
{
    constexpr int error = -1;
    int status = error;
    int sock_fd = socket(PF_PACKET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, htons(ETH_P_ALL));
    if (sock_fd < 0)
    {
        fprintf(stderr, "Can't create raw socket!\n");
        return error;
    }

    // Not bind socket to interface with empty name.
    if (!ifaceName.empty())
    {
        // Binding socket to an interface.
        status = setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, ifaceName.c_str(), ifaceName.length() + 1);
        if (status < 0)
        {
            fprintf(stderr, "Can't bind a socket to specified interface!\n");
            close(sock_fd);
            return error;
        }
    }

    /* Set the network card in promiscuos mode.
     * An ioctl() request has encoded in it whether the argument is an in parameter or out parameter.
     * SIOCGIFFLAGS	0x8913 - get flags;
     * SIOCSIFFLAGS	0x8914 - set flags.
     */
    struct ifreq ethreq;
    strncpy(ethreq.ifr_name, ifaceName.c_str(), IF_NAMESIZE);
    status = ioctl(sock_fd, SIOCGIFFLAGS, &ethreq);
    if (status == error)
    {
        fprintf(stderr, "Can't get socket flags with ioctl!\n");
        close(sock_fd);
        return error;
    }
    ethreq.ifr_flags |= IFF_PROMISC;
    status = ioctl(sock_fd, SIOCSIFFLAGS, &ethreq);
    if (status == error)
    {
        fprintf(stderr, "Can't set socket flags with ioctl!\n");
        close(sock_fd);
        return error;
    }
    return sock_fd;
}

void BpfWrapper::filterSourceCode(const std::string &iface, const std::string &srcMac, const std::string &svID)
{
    constexpr static auto svReplacement1 = R"(char cmp_str[] = "%SV_ID";)";
    constexpr static auto svReplacement2 = R"(
        if (sizeof(cmp_str) != length)
            return false;
    )";

    bpfSrc.replace("%IFACE_MAC", util::get_mac_by_iface_name(iface));
    bpfSrc.replace("%SRC_MAC", srcMac);
    if (!svID.empty())
        bpfSrc.replace("%SV_ID", svID);
    else
    {
        bpfSrc.replace(svReplacement1, "char const *cmp_str = str;");
        bpfSrc.remove(svReplacement2);
    }

    /// TODO: replace it
    std::cout << "code:\n\n" << bpfSrc.getSourceCode() << "\n\n";
}

ebpf::StatusTuple BpfWrapper::run()
{
    return bpfPtr->init(bpfSrc.getSourceCode());
}

ebpf::BPF *BpfWrapper::getBpfObject()
{
    return bpfPtr.get();
}

ebpf::StatusTuple BpfWrapper::attachRawSocket(const std::string &ifaceName, const int function, int &socket)
{
    socket = get_raw_socket(ifaceName);
    if (socket >= 0)
    {
        auto result = bpf_attach_socket(socket, function);
        if (result >= 0)
            return ebpf::StatusTuple::OK();
        else
        {
            close(socket);
            return ebpf::StatusTuple(-1, "bpf: Failed attaching function %i to socket", function);
        }
    }
    else
        return ebpf::StatusTuple(-1, "bpf: Failed open socket for device %s", ifaceName.c_str());
}
