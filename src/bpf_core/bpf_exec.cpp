#include "bpf_core/bpf_exec.h"

#include <bpf_core/utils/helpers.h>
#include <cstdio>
#include <fstream>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/perf_event.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

BpfExec::BpfExec(const std::string &programPath) : bpfPtr(new ebpf::BPF), bpfProg(programPath)
{
}

int BpfExec::getRawSocket(const std::string &ifaceName)
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
    strncpy(ethreq.ifr_name, ifaceName.c_str(), IF_NAMESIZE - 1);
    // Stringop-truncation fix with adding a null-term last character
    ethreq.ifr_name[IF_NAMESIZE - 1] = '\0';
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

void BpfExec::filterSourceCode(const std::string &ifaceName, const std::string &srcMac, const std::string &svID)
{
    constexpr static auto svReplacement1 = R"(char cmp_str[] = "%SV_ID";)";
    constexpr static auto svReplacement2 = R"(
        if (sizeof(cmp_str) != length)
            return false;
    )";

    bpfProg.replace("%IFACE_MAC", utils::get_mac_by_iface_name(ifaceName));
    bpfProg.replace("%SRC_MAC", srcMac);
    if (!svID.empty())
        bpfProg.replace("%SV_ID", svID);
    else
    {
        bpfProg.replace(svReplacement1, "char const *cmp_str = str;");
        bpfProg.remove(svReplacement2);
    }

    /// TODO: replace it
    // std::cout << "code:\n\n" << bpfProg.getSourceCode() << "\n\n";
}

ebpf::StatusTuple BpfExec::run()
{
    return bpfPtr->init(bpfProg.getSourceCode(), { "-Wno-macro-redefined" });
}

ebpf::StatusTuple BpfExec::attachRawSocket(const std::string &ifaceName, const int function, int &socket)
{
    socket = getRawSocket(ifaceName);
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

ebpf::StatusTuple BpfExec::getDeviceSocket(int &sock_fd, const std::string &functionName, const std::string &ifaceName)
{
    int fd_func = -1;
    auto status = bpfPtr->load_func(functionName, BPF_PROG_TYPE_SOCKET_FILTER, fd_func);
    if (status.ok())
    {
        // name of device may be eth0, eth1, etc (see 'ifconfig' or 'ip a')...
        status = attachRawSocket(ifaceName, fd_func, sock_fd);
    }
    return status;
}
