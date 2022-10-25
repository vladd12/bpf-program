#pragma once

#include <cstdint>
#include <ctime>
#include <linux/bpf.h>
#include <linux/if_link.h>
#include <linux/perf_event.h>
#include <linux/time.h>

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

constexpr clockid_t ClockMono = CLOCK_MONOTONIC;

void test()
{
    timespec val;
    if (clock_gettime(ClockMono, &val) == 0)
    {
        // its okay
    }
    else
    {
        // its not okay
    }
}

} // namespace types
