#pragma once

#include <bcc/BPF.h>
#include <bpf/libbpf.h>
#include <fstream>
// temp
#include <iostream>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;

    auto read_file(const std::string_view path) -> std::string
    {
        constexpr auto read_size = std::size_t(4096);
        auto stream = std::ifstream(path.data());
        stream.exceptions(std::ios_base::badbit);

        auto out = std::string();
        auto buf = std::string(read_size, '\0');
        while (stream.read(&buf[0], read_size))
            out.append(buf, 0, stream.gcount());

        out.append(buf, 0, stream.gcount());
        return out;
    }

public:
    explicit BpfWrapper() : bpfPtr(new ebpf::BPF)
    {
    }

    ebpf::StatusTuple initByFile(const std::string_view filename)
    {
        auto bpfProgText = read_file(filename);
        std::cout << "\n\n\nProgram text:\n" << bpfProgText << '\n';
        return bpfPtr->init(bpfProgText);
    }

    ebpf::BPF *getBpfObject()
    {
        return bpfPtr.get();
    }

    ebpf::StatusTuple attach_raw_socket(const std::string &device, const int function, int &socket)
    {
        socket = bpf_open_raw_sock(device.c_str());
        if (socket >= 0)
        {
            auto result = bpf_attach_socket(socket, function);
            if (result >= 0)
                return ebpf::StatusTuple::OK();
            else
                return ebpf::StatusTuple(-1, "Failed attaching function to socket");
        }
        else
            return ebpf::StatusTuple(-1, "Failed open socket for device %s", device.c_str());
    }
};
