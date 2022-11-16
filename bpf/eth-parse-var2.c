#include <linux/sched.h>
#include <linux/skbuff.h>
#include <uapi/linux/ptrace.h>
#include <bcc/proto.h>

struct xmit_event {
    u32 len;
    u32 datalen;
    u32 packet_buf_ptr;
    u32 reserved;
};
BPF_PERF_OUTPUT(xmits);

#define PACKET_BUF_SIZE 2048
struct packet_buf {
    u8 data[PACKET_BUF_SIZE];
};

#define PACKET_BUFS_PER_CPU 1
BPF_PERCPU_ARRAY(packet_buf, struct packet_buf, PACKET_BUFS_PER_CPU);
BPF_PERCPU_ARRAY(packet_buf_head, u32, PACKET_BUFS_PER_CPU);

#define IEC_PROTO 0x8100    // 0x8100 - IEC 61850 protocol
#define SV_PRIORITY 0b100   // SV protocol user priority
#define SV_ETHERTYPE 0x88BA // SV message identifier

int kprobe____dev_queue_xmit(struct pt_regs *ctx, struct sk_buff *skb, void *accel_priv) {
    if (skb == NULL || skb->data == NULL)
        return 0;

    struct xmit_event data = { };
    data.len = skb->len;

    // Copy packet contents
    int slot = 0;
    u32 *packet_buf_ptr = packet_buf_head.lookup(&slot);
    if (packet_buf_ptr == NULL)
        return 0;

    u32 buf_head = *packet_buf_ptr;
    u32 next_buf_head = (buf_head + 1) % PACKET_BUFS_PER_CPU;
    packet_buf_head.update(&slot, &next_buf_head);

    struct packet_buf *ringbuf = packet_buf.lookup(&buf_head);
    if (ringbuf == NULL)
        return 0;

    u32 skb_data_len = skb->data_len;
    u32 headlen = data.len - skb_data_len;
    headlen &= 0xffffff; // Useless, but validator demands it because "this unsigned(!) variable could otherwise be negative"
    bpf_probe_read_kernel(ringbuf->data, headlen < PACKET_BUF_SIZE ? headlen : PACKET_BUF_SIZE, skb->data);
    
    // Filtering
    u8 *cursor = &(ringbuf->data[0]);
    
    struct ethernet_t *ethernet = (struct ethernet_t *)cursor;
    // filter frames with protocol type
    if (ethernet->type != IEC_PROTO) {
        return 0;
    }
    cursor = cursor + sizeof(*ethernet);

    // filter frames with SV user priority
    struct dot1q_t *iec = (struct dot1q_t *)cursor;
    if (iec->pri != SV_PRIORITY) {
        return 0;
    }

    // filter frames with SV message identifier
    if (iec->type != SV_ETHERTYPE) {
        return 0;
    }
    cursor = cursor + sizeof(*iec);
    
    data.packet_buf_ptr = buf_head;
    data.len = headlen;
    data.datalen = skb_data_len;

    xmits.perf_submit(ctx, &data, sizeof(data));

    return 0;
}

