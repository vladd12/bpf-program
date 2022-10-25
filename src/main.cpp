#include <BpfWrapper.h>
#include <iostream>

void inline printErrorMsg(const ebpf::StatusTuple &status)
{
    std::cout << status.msg() << '\n';
}

int main()
{
    using namespace std;
    auto bpf = std::unique_ptr<BpfWrapper>(new BpfWrapper);
    auto executeStatus = bpf->initByFile("./bpf/ethernet-parse");
    if (executeStatus.ok())
    {
        int fd_func = -1;
        auto bpfObject = bpf->getBpfObject();
        executeStatus = bpfObject->load_func("bpf", BPF_PROG_TYPE_SOCKET_FILTER, fd_func);
        if (executeStatus.ok())
        {
            int sock_fd = -1;
            executeStatus = bpf->attach_raw_socket("eth0", fd_func, sock_fd);
            if (executeStatus.ok())
            {
                ;
            }
            else
                printErrorMsg(executeStatus);
        }
        else
            printErrorMsg(executeStatus);
    }
    else
        printErrorMsg(executeStatus);

    return 0;
}
