#pragma once

#include <bcc/BPF.h>
#include <common.h>
#include <functional>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;

    /// \brief Reading file specified by filepath in std::string.
    std::string read_file(const std::string_view &filepath);

    /// \brief Replacing all substrings "what" with "with".
    std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with);

    /// \brief Removing all substrings "what".
    std::size_t remove_all(std::string &inout, std::string_view what);

    ebpf::StatusTuple socket_configure(int &socket, const char *ifaceName);

public:
    /// \brief Default c-tor.
    explicit BpfWrapper();

    /// \brief Initializations eBPF program from file specified by filepath.
    ebpf::StatusTuple initByFile(const std::string_view &programPath);

    /// \brief Returns eBPF object stored in ebpf::BPF smart pointer.
    ebpf::BPF *getBpfObject();

    ebpf::StatusTuple openPerfBuf(const std::string_view bufName, std::function<void(void *, void *, int)> bufReader);

    ebpf::StatusTuple closePerfBuf(const std::string_view bufName);

    /// \brief Attaching raw socket for listening ethernbet device, specified by device name.
    ebpf::StatusTuple attachRawSocket(const std::string &deviceName, const int function, int &socket);
};
