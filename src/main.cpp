#include <bcc/BPF.h>
#include <iostream>
#include <memory>

int main() {
  using namespace std;
  using bpf = ebpf::BPF;

  auto bpfObject = std::unique_ptr<bpf>(new bpf);
  constexpr auto bpf_text = " \
  #include <uapi/linux/ptrace.h> \
  #include <net/sock.h> \
  #include <bcc/proto.h> \
  BPF_HASH(currsock, u32, struct sock *); \
      int kprobe__tcp_v4_connect(struct pt_regs * ctx, struct sock * sk) { \
    u32 pid = bpf_get_current_pid_tgid(); \
    currsock.update(&pid, &sk); \
    return 0; \
  }; \
  int kretprobe__tcp_v4_connect(struct pt_regs * ctx) { \
    int ret = PT_REGS_RC(ctx); \
    u32 pid = bpf_get_current_pid_tgid(); \
    struct sock **skpp; \
    skpp = currsock.lookup(&pid); \
    if (skpp == 0) { \
      return 0;  \
    } \
    if (ret != 0) { \
      currsock.delete(&pid); \
      return 0; \
    } \
    struct sock *skp = *skpp; \
    u32 saddr = skp->__sk_common.skc_rcv_saddr; \
    u32 daddr = skp->__sk_common.skc_daddr; \
    u16 dport = skp->__sk_common.skc_dport; \
    bpf_trace_printk(\"trace_tcp4connect %x %x %d\\n\", saddr, daddr, \
                     ntohs(dport)); \
    currsock.delete(&pid); \
    return 0; \
  } \
  ";
  bpfObject->init(bpf_text);

  /*
    while (true) {
      ;
    }
   */

  return 0;
}
