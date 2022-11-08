#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>

#define IEC_PROTO 0x8100    // 0x8100 - IEC 61850 protocol
#define SV_PRIORITY 0b100   // SV protocol user priority
#define SV_ETHERTYPE 0x88BA // SV message identifier

/* eBPF program.
   Filter IEC 61850-9-2 SV frames, having payload not empty if the program
   is loaded as PROG_TYPE_SOCKET_FILTER and attached to a socket.
   return  0 -> DROP the frame.
   return -1 -> KEEP the frame and return it to user space (userspace can read it from the socket_fd).
*/
int iec61850_filter(struct __sk_buff *skb) {
    u8 *cursor = 0;
    // current ethernet device MAC address
    // NOTE: value RECEIVER_MAC declared by user programm
    u64 receiverMac = RECEIVER_MAC;

    struct ethernet_t *ethernet = cursor_advance(cursor, sizeof(*ethernet));
    u64 destinationMac = ethernet->dst;

    // filter frames with destination MAC address
    if (receiverMac != destinationMac) {
        goto DROP;
    }

    // filter frames with protocol type
    if (ethernet->type != IEC_PROTO) {
        goto DROP;
    }

    // filter frames with SV user priority
    struct dot1q_t *iec = cursor_advance(cursor, sizeof(*iec));
    if (iec->pri != SV_PRIORITY) {
        goto DROP;
    }

    // filter frames with SV message identifier
    if (iec->type != SV_ETHERTYPE) {
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
