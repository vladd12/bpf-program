#include "bpf_core/net/pcap-driver.h"

#include <pcap.h>

namespace net
{

PcapDriver::PcapDriver()
{
    char error_buffer[PCAP_ERRBUF_SIZE]; /* Size defined in pcap.h */
    auto iface_list = new pcap_if_t[5];

    /* Find a device */
    auto status = pcap_findalldevs(&iface_list, &error_buffer[0]);
    if (status == PCAP_ERROR)
    {
        printf("Error finding device: %s\n", error_buffer);
    }
    pcap_freealldevs(iface_list);
    // printf("Network device found: %s\n", device);
}

}
