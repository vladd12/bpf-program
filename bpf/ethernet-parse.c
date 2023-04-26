#include <uapi/linux/ptrace.h>
#include <net/sock.h>
#include <bcc/proto.h>
#include <bcc/helpers.h>

#define IEC_PROTO 0x8100      // 0x8100 - IEC 61850 protocol
#define SV_PRIORITY 0b100     // SV protocol user priority
#define SV_ETHERTYPE 0x88BA   // SV message identifier
#define SV_PDU_ID_80P 0x60    // SV PDU ID for frames with 80 data points
#define SV_PDU_ID_256P 0x6082 // SV PDU ID for frames with 256 data points

BPF_RINGBUF_OUTPUT(buffer, 32);

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

struct sv_seqASDU_256p {
    u16 seqAsduId;
    u16 length;
} BPF_PACKET_HEADER;

struct sv_noASDU_256p {
    u8 noSduId;
    u8 length;
    u8 blockNum;
    struct sv_seqASDU_256p seqASDU;
} BPF_PACKET_HEADER;

struct sv_savPDU_256p {
    u16 savPduId;
    u16 length;
    struct sv_noASDU_256p noASDU;
} BPF_PACKET_HEADER;

struct ASDU_start_t {
    u8 asduId;
    u8 length;
    u8 svId;
    u8 svIdLength;
} BPF_PACKET_HEADER;

// String comparing in BPF program
static inline bool SvIdCmp(void* data, unsigned long length) {
    // NOTE: value SV_ID declared by user programm
    char cmp_str[] = "%SV_ID";

    if ((sizeof(cmp_str) - 1) != length)
        return false;
    
    char byte = 0;
    for (unsigned long i = 0; i < sizeof(cmp_str) - 1; ++i) {
        byte = *((char*)(data + i));
        if (byte  != cmp_str[i]) {
            return false;
        }
    }
    return true;
}

/* eBPF program.
   Filter IEC 61850-9-2 SV frames, having payload not empty if the program
   is loaded as PROG_TYPE_SOCKET_FILTER and attached to the socket.
   return  0 -> DROP the frame.
   return -1 -> KEEP the frame and return it to user space (userspace can read it from the socket_fd).
*/
static bool iec61850_filter(struct sk_buff *skb) {
    u8 *cursor = 0;
    void *p = (void *)(long)skb->data;

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
    unsigned long pduIdOffset = sizeof(*ethernet) + sizeof(*sv_start);
    // Read PDU ID from skb
    u16 pduId256 = load_half(skb, pduIdOffset);
    u16 pduId80 = (pduId256 >> 8) & 0x00FF;
    bpf_trace_printk("0x%x 0x%x\n", pduId256, pduId80);

    unsigned long pduSize = 0;
    // Frame with 256 data points
    if (pduId256 == SV_PDU_ID_256P) {
        struct sv_savPDU_256p *savPDU = cursor_advance(cursor, sizeof(*savPDU));
        pduSize = sizeof(*savPDU);
    }
    // Frame with 80 data points
    else if (pduId80 == SV_PDU_ID_80P) {
        struct sv_savPDU_80p *savPDU = cursor_advance(cursor, sizeof(*savPDU));
        pduSize = sizeof(*savPDU);
    }
    // Unknown data
    else {
        goto DROP;
    }

    // Calculating offset and getting length of sample values ID.
    struct ASDU_start_t *asduHead = cursor_advance(cursor, sizeof(*asduHead));
    const unsigned long svIdOfsset = pduIdOffset + pduSize + sizeof(*asduHead);
    void* start = ((void *)(long)skb->data) + svIdOfsset;
    const u8 svIdLength = asduHead->svIdLength;

    // Comparing sample values ID.
    bool cmp = SvIdCmp(start, svIdLength);
    if (!cmp) {
        // bpf_trace_printk("It's not Ok :(");
        goto DROP;
    }

    goto KEEP;

    // keep the packet and send it to userspace: returning -1
    KEEP:
        return 1;

    // drop the packet: returning 0
    DROP:
        return 0;
}

TRACEPOINT_PROBE(net, net_dev_start_xmit){
    struct sk_buff skb;
    bpf_probe_read(&skb, sizeof(skb), args->skbaddr);
    if (iec61850_filter(&skb)) {
        ;
    }
    return 0;
}



