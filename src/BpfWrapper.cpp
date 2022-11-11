#include <BpfWrapper.h>
#include <bpf/libbpf.h>
#include <fstream>
#include <net/if.h>
#include <sys/ioctl.h>

BpfWrapper::BpfWrapper() : bpfPtr(new ebpf::BPF)
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

ebpf::StatusTuple BpfWrapper::socket_configure(int &socket, const char *ifaceName)
{
    ifreq req;
    memset(&req, 0, sizeof(req));
    strncpy(req.ifr_name, ifaceName, IF_NAMESIZE);
    if (ioctl(socket, SIOCGIFFLAGS, &req) >= 0)
    {
        req.ifr_flags |= IFF_PROMISC;
        if (ioctl(socket, SIOCSIFFLAGS, &req) >= 0)
        {
            if (ioctl(socket, SIOCGIFINDEX, &req) >= 0)
                return ebpf::StatusTuple::OK();
            else
                return ebpf::StatusTuple(-1, "bpf: Failed setting 'SIOCGIFINDEX' I/O Control flag for socket");
        }
        else
            return ebpf::StatusTuple(-1, "bpf: Failed setting 'SIOCSIFFLAGS' I/O Control flag for socket");
    }
    else
        return ebpf::StatusTuple(-1, "bpf: Failed setting 'SIOCGIFFLAGS' I/O Control flag for socket");
}

ebpf::StatusTuple BpfWrapper::initByFile(const std::string_view &programPath)
{
    auto bpfProgText = read_file(programPath);
    return bpfPtr->init(bpfProgText);
}

ebpf::BPF *BpfWrapper::getBpfObject()
{
    return bpfPtr.get();
}

ebpf::StatusTuple BpfWrapper::openPerfBuf(const std::string_view bufName, std::function<void(void *, void *, int)> bufReader)
{
    return bpfPtr->open_perf_buffer(bufName.data(), bufReader.target<void(void *, void *, int)>());
}

ebpf::StatusTuple BpfWrapper::closePerfBuf(const std::string_view bufName)
{
    return bpfPtr->close_perf_buffer(bufName.data());
}

ebpf::StatusTuple BpfWrapper::attachRawSocket(const std::string &deviceName, const int function, int &socket)
{
    socket = bpf_open_raw_sock(deviceName.c_str());
    if (socket >= 0)
    {
        auto configureStatus = socket_configure(socket, deviceName.c_str());
        if (configureStatus.ok())
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
        {
            close(socket);
            return configureStatus;
        }
    }
    else
        return ebpf::StatusTuple(-1, "bpf: Failed open socket for device %s", deviceName.c_str());
}
