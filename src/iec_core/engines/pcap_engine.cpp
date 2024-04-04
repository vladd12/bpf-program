#include "iec_core/engines/pcap_engine.h"

#include <pcap.h>

namespace engines
{

PCAPEngine::PCAPEngine()
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

} // namespace engines
