#define KBUILD_MODNAME "foo"
#include <uapi/linux/bpf.h>
#include <uapi/linux/in.h>
#include "bpf_helpers.h"

#define STYPE_IDX 54
#define PROBE_RESP_CODE 0x50
#define BEACON_CODE 0x88

#define SAMPLE_SIZE 512ul
#define MAX_CPUS 128
#define COOKIE 0xdead

#define FILTER_2_4GHZ 0x1
#define FILTER_5_GHZ 0x2

struct bpf_map_def SEC("maps") packet_map = {
	.type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,
	.key_size = sizeof(int),
	.value_size = sizeof(u32),
	.max_entries = MAX_CPUS,
};

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(key_size, sizeof(int));
	__uint(value_size, sizeof(int));
	__uint(max_entries, 1);
} filter_map SEC(".maps");

SEC("xdp_scanner")
int xdp_sample_prog(struct xdp_md *ctx)
{
	int key = 1;
	void *data_end = (void *)(long)ctx->data_end;
	void *data = (void *)(long)ctx->data;
	u8 *frame = data;

	struct S {
		u16 cookie;
		u16 pkt_len;
	} __packed metadata;

  	if (frame + STYPE_IDX + 1 > data_end)
    		return XDP_PASS;

	if (frame[STYPE_IDX] != PROBE_RESP_CODE && frame[STYPE_IDX] != BEACON_CODE)
		return XDP_PASS;

	if (data < data_end)
	{
		u64 flags = BPF_F_CURRENT_CPU;
		u16 sample_size;
		int ret, *filter;

		filter = bpf_map_lookup_elem(&filter_map, &key);

		if (filter)
		{
			__u16 freq = ((__u16 *)frame)[13];

			if ((*filter & FILTER_2_4GHZ && freq > 3000) || (*filter & FILTER_5_GHZ && freq < 3000))
				return XDP_PASS;
		}

		metadata.cookie = COOKIE;
		metadata.pkt_len = (u16)(data_end - data);
		sample_size = min(metadata.pkt_len, SAMPLE_SIZE);
		flags |= (u64)sample_size << 32;

		if (bpf_perf_event_output(ctx, &packet_map, flags, &metadata, sizeof(metadata)))
			bpf_printk("perf_event_output failed\n");
	}

	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
