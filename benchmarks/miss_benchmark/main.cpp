//#include <bpf_exec.h>
//#include <chrono>
//#include <iostream>
//#include <net/sock.h>
//#include <net/validator.h>
//#include <thread>
//#include <utility>

#include "packet_counter.h"

#include <memory>

int main()
{
    using namespace std::chrono_literals;
    using namespace detail;

    std::unique_ptr<PacketCounter> pCounter(new PacketCounter("enp0s8", "0x0cefaf3042cc", "ENS80pointMU01"));
    pCounter->readInTime(1000ms, TargetSocket::Native);
    pCounter->readInTime(1000ms, TargetSocket::BPF);
    return 0;
}
