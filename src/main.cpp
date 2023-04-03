#include <bpf_exec.h>
#include <exception>
#include <fast_file.h>
#include <iostream>
#include <utils.h>

// Linux
#include <net/ethernet.h>
#include <sys/socket.h>

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;

constexpr inline word makeword(const byte &lhs, const byte &rhs)
{
    return (lhs << 8 | rhs);
}

void test(int &sock)
{
    constexpr std::size_t bufSize = 2048;
    constexpr std::size_t smpCnt80pOffset = 35;
    constexpr word smpCntMax = 3999;

    word min = UINT16_MAX, max = 0, prev = 0, curr = 1;
    bool firstTime = true;
    auto buffer = new byte[bufSize];
    auto file = FastFile("out.txt");
    printf("Start work\n");

    while (true)
    {
        auto rcStat = recvfrom(sock, buffer, bufSize, 0, nullptr, nullptr);
        if (rcStat >= 0)
        {
            if (!firstTime)
                prev = curr;

            auto iter = buffer;
            iter += sizeof(ether_header);
            iter += smpCnt80pOffset;
            auto smpCnt1 = reinterpret_cast<byte *>(iter++);
            auto smpCnt2 = reinterpret_cast<byte *>(iter++);
            curr = makeword(*smpCnt1, *smpCnt2);
            printf("Count: %04X\n", curr);

            if (!firstTime)
            {
                if (curr == 0)
                {
                    if (prev != smpCntMax)
                        // throw std::runtime_error("Packet missed")
                        ;
                }
                else
                {
                    auto offset = curr - prev;
                    if (offset > 1)
                        // throw std::runtime_error("Packet missed")
                        ;
                }
                // file.write(curr);
            }

            if (curr > max)
                max = curr;
            if (curr < min)
                min = curr;
            if (firstTime)
                firstTime = false;
            // if (max > curr && min < curr)
            //    break;
        }
    }
    delete[] buffer;
    std::cout << "Min: " << min << "\nMax: " << max << "\n\n";
}

void inputData(std::string &iface, std::string &srcMac, std::string &svID)
{
    std::cout << "Enter the ethernet interface name: ";
    std::cin >> iface;
    std::cout << "Enter the source MAC address (format: 0xFFFFFFFFFFFF): ";
    std::cin >> srcMac;
    std::cout << "Enter the sample values ID: ";
    std::cin >> svID;
    // test
    std::cout << "Test input: " << iface << ' ' << srcMac << ' ' << svID << '\n';
}

int main()
{
    auto bpf = std::unique_ptr<BpfExec>(new BpfExec("bpf/ethernet-parse.c"));
    auto ifaceName = std::string("enp0s8");          // enp0s8 - VM, eth0 - hardware
    auto srcMacAddr = std::string("0x0cefaf3042cc"); // 0x0cefaf3042cc - 80p, 0x0cefaf3042cd - 256p
    auto svID = std::string("ENS80pointMU01");       // ENS80pointMU01 - 80p, ENS256MUnn01 - 256p
    inputData(ifaceName, srcMacAddr, svID);
    bpf->filterSourceCode(ifaceName, srcMacAddr, svID);

    auto status = bpf->run();
    if (status.ok())
    {
        int sock = -1;
        status = bpf->getDeviceSocket(sock, "iec61850_filter", ifaceName);
        if (status.ok() && sock >= 0)
        {
            test(sock);
        }
    }
    util::printStatusMessage(status);
    return 0;
}
