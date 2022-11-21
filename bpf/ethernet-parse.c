#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>
#include <bcc/helpers.h>

#define IEC_PROTO 0x8100    // 0x8100 - IEC 61850 protocol
#define SV_PRIORITY 0b100   // SV protocol user priority
#define SV_ETHERTYPE 0x88BA // SV message identifier

struct sv_start_t {
    u16 appid;
    u16 length;
    u16 reserved1;
    u16 reserved2;
} BPF_PACKET_HEADER;

struct sv_seqASDU_80p {
    u8 seqAsduId;
    u8 length;
} BPF_PACKET_HEADER;

struct sv_noASDU_80p {
    u8 noSduId;
    u8 length;
    u8 blockNum;
    struct sv_seqASDU_80p seqASDU;
} BPF_PACKET_HEADER;

struct sv_savPDU_80p {
    u8 savPduId;
    u8 length;
    struct sv_noASDU_80p noASDU;
} BPF_PACKET_HEADER;

struct ASDU_start_t {
    u8 asduId;
    u8 length;
    u8 svId;
    u8 svIdLength;
} BPF_PACKET_HEADER;

static inline bool SV_CMP(char const *str, unsigned long length) {
    // NOTE: value SV_ID declared by user programm
    char cmp_str[] = "%SV_ID";

    if ((sizeof(cmp_str) - 1) != length)
        return false;

    bpf_trace_printk("%u %u", sizeof(cmp_str) - 1, length);

    char comparand[sizeof(cmp_str) - 1];
    long err = bpf_probe_read_kernel(&comparand, sizeof(comparand), str);
    if (err) {
        bpf_trace_printk("Error: %ld", err);
    }
    
    for (unsigned long i = 0; i < length; ++i) {
        bpf_trace_printk("%u %u", cmp_str[i], comparand[i]);
        if (cmp_str[i] != comparand[i]) {
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

    struct sv_start_t *sv_start = cursor_advance(cursor, sizeof(*sv_start));
    struct sv_savPDU_80p *savPDU = cursor_advance(cursor, sizeof(*savPDU));
    struct ASDU_start_t *asduHead = cursor_advance(cursor, sizeof(*asduHead));
    const u8 len = asduHead->svIdLength;
    char *svID = cursor_advance(cursor, len);
    if (svID == NULL) {
        goto DROP;
    }

    //unsigned char svIdName[255];
    //bpf_probe_read_kernel(&svIdName[0], len, cursor);

    bool cmp = SV_CMP(svID, len);
    if (cmp == true) {
        bpf_trace_printk("It's Ok");
    }
    else {
        bpf_trace_printk("It's not Ok :(");
    }


    // bpf_trace_printk("0x%x", asduHead->svIdLength);

    goto KEEP;

    // keep the packet and send it to userspace: returning -1
    KEEP:
        return -1;

    // drop the packet: returning 0
    DROP:
        return 0;
}

