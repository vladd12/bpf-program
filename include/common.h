#pragma once

#include <bcc/bcc_exception.h>
#include <cstdint>
#include <iostream>
#include <linux/bpf.h>
#include <linux/if_link.h>
#include <linux/perf_event.h>

namespace types
{

using PerfType = perf_type_id;
using PerfHWConfig = perf_hw_id;
using PerfSWConfig = perf_sw_ids;
using PerfEventSampleFormat = perf_event_sample_format;
using BpfProgType = bpf_prog_type;
using BpfAttachType = bpf_attach_type;
using XdpAction = xdp_action;

enum XdpFlags : std::uint8_t
{
    UPDATE_IF_NOEXIST = XDP_FLAGS_UPDATE_IF_NOEXIST,
    SKB_MODE = XDP_FLAGS_SKB_MODE,
    DRV_MODE = XDP_FLAGS_DRV_MODE,
    HW_MODE = XDP_FLAGS_HW_MODE,
    REPLACE = XDP_FLAGS_REPLACE
};

} // namespace types

void inline printStatusMsg(const ebpf::StatusTuple &status)
{
    std::cout << status.msg() << '\n';
}
