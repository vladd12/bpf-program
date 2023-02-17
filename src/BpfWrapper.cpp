#include <BpfWrapper.h>
//#include <bpf/libbpf.h>
#include <cstdio>
#include <fstream>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

const static auto svReplacement1 = R"(char cmp_str[] = "%SV_ID";)";
const static auto svReplacement2 = R"(
    if (sizeof(cmp_str) != length)
        return false;
)";

BpfWrapper::BpfWrapper() : bpfPtr(new ebpf::BPF), bpfProg()
{
}

std::string BpfWrapper::read_file(const std::string_view &filepath)
{
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(filepath.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size))
        out.append(buf, 0, stream.gcount());

    out.append(buf, 0, stream.gcount());
    return out;
}

std::size_t BpfWrapper::replace_all(std::string &inout, const std::string_view what, const std::string_view with)
{
    auto count = std::size_t(0);
    for (auto pos = std::string::size_type {};                              //
         inout.npos != (pos = inout.find(what.data(), pos, what.length())); //
         pos += with.length(), ++count)                                     //
    {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

std::size_t BpfWrapper::remove_all(std::string &inout, const std::string_view what)
{
    return replace_all(inout, what, "");
}

void BpfWrapper::filter_iface_mac(const std::string &ifaceName)
{
    // Replacing ethernet interface MAC address
    constexpr auto ifacePlaceholder = "%IFACE_MAC";
    if (!ifaceName.empty())
    {
        auto deviceAddressFilepath = "/sys/class/net/" + ifaceName + "/address";
        auto addressText = read_file(deviceAddressFilepath);
        remove_all(addressText, ":");
        remove_all(addressText, "\n");
        replace_all(bpfProg, ifacePlaceholder, "0x" + addressText);
    }
    else
        replace_all(bpfProg, ifacePlaceholder, "0xFFFFFFFFFFFF");
}

void BpfWrapper::filter_src_mac(const std::string &srcMac)
{
    // Replacing remote device MAC address
    constexpr auto srcMacPlaceholder = "%SRC_MAC";
    if (!srcMac.empty())
        replace_all(bpfProg, srcMacPlaceholder, srcMac);
    else
        replace_all(bpfProg, srcMacPlaceholder, "sourceMac");
}

void BpfWrapper::filter_sv_id(const std::string &svID)
{
    // Replacing sample values ID with specified
    constexpr auto svIdPlaceholder = "%SV_ID";
    if (!svID.empty())
        replace_all(bpfProg, svIdPlaceholder, svID);
    else
    {
        replace_all(bpfProg, svReplacement1, "char const *cmp_str = str;");
        remove_all(bpfProg, svReplacement2);
    }
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

void BpfWrapper::initByFile(const std::string &programPath)
{
    bpfProg = read_file(programPath);
}

ebpf::StatusTuple BpfWrapper::filterProgText(const std::string &iface, const std::string &srcMac, const std::string &svID)
{
    filter_iface_mac(iface);
    filter_src_mac(srcMac);
    filter_sv_id(svID);
    std::cout << "code:\n\n" << bpfProg << "\n\n";
    return bpfPtr->init(bpfProg);
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
