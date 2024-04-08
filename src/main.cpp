#include <iec_core/engines/bpf_engine.h>
#include <iec_core/engines/bpf_exec.h>
#include <iec_core/engines/pcap_engine.h>
#include <iostream>
#include <string>

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
    std::string ifaceName = "enp0s8";          // enp0s8 - VM, eth0 - hardware
    std::string srcMacAddr = "0x0cefaf3042cc"; // 0x0cefaf3042cc - 80p, 0x0cefaf3042cd - 256p
    std::string svID = "ENS80pointMU01";       // ENS80pointMU01 - 80p, ENS256MUnn01 - 256p
    // inputData(ifaceName, srcMacAddr, svID);

    engines::BPFEngine engine;
    if (engine.setup({ ifaceName, srcMacAddr, svID }))
        engine.run();

    return 0;
}
