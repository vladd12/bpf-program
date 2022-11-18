#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>

#define IEC_PROTO 0x8100    // 0x8100 - IEC 61850 protocol
#define SV_PRIORITY 0b100   // SV protocol user priority
#define SV_ETHERTYPE 0x88BA // SV message identifier

static inline bool SV_CMP(char const *str, unsigned long length) {
    // NOTE: value SV_ID declared by user programm
    char cmp_str[] = "%SV_ID";
    if (sizeof(cmp_str) != length)
        return false;
    
    for (unsigned long i = 0; i < length; ++i) {
        if (cmp_str[i] != str[i]) {
            return false;
        }
    }
    return true;
}

/* eBPF program.
   Filter IEC 61850-9-2 SV frames, having payload not empty if the program
   is loaded as PROG_TYPE_SOCKET_FILTER and attached to a socket.
   return  0 -> DROP the frame.
   return -1 -> KEEP the frame and return it to user space (userspace can read it from the socket_fd).
*/
int iec61850_filter(struct __sk_buff *skb) {
    u8 *cursor = 0;

    // Source MAC address from capturing ethernet frame
    struct ethernet_t *ethernet = cursor_advance(cursor, sizeof(*ethernet));
    u64 sourceMac = ethernet->src;

    // Filter frames with protocol type
    if (ethernet->type != IEC_PROTO && ethernet->type != SV_ETHERTYPE) {
        goto DROP;
    }

    // Current ethernet device MAC address
    // NOTE: value IFACE_MAC declared by user programm
    u64 ifaceMac = %IFACE_MAC;

    // Filter frames that socket send
    if (ifaceMac == sourceMac) {
        goto DROP;
    }
    
    // Sender ethernet device MAC address
    // NOTE: value SRC_MAC declared by user programm
    u64 specMac = %SRC_MAC;
    
    // Filter frames with specified source MAC address
    if (specMac != sourceMac) {
        goto DROP;
    }

    goto KEEP;

    // keep the packet and send it to userspace: returning -1
    KEEP:
        return -1;

    // drop the packet: returning 0
    DROP:
        return 0;
}

