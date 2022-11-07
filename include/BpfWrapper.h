#pragma once

#include <bcc/BPF.h>
#include <common.h>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;

    /// \brief Reading file specified by filepath in std::string.
    std::string read_file(const std::string_view &filepath);

    std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with);

    std::size_t remove_all(std::string &inout, std::string_view what);

public:
    /// \brief Default c-tor.
    explicit BpfWrapper();

    /// \brief Initializations eBPF program from file specified by filepath.
    ebpf::StatusTuple initByFile(const std::string_view &programPath, const std::string_view &deviceName);

    /// \brief Returns eBPF object stored in ebpf::BPF smart pointer.
    ebpf::BPF *getBpfObject();

    /// \brief Attaching raw socket for listening ethernbet device, specified by device name.
    ebpf::StatusTuple attachRawSocket(const std::string &deviceName, const int function, int &socket);
};
