#ifndef _XDP_WLSCAN_BPF_UTILS_H_
#define _XDP_WLSCAN_BPF_UTILS_H_

#define MON_INTF "mon0"

#ifdef __cplusplus
extern "C"
{
#endif
	extern int if_idx;
	extern char *if_name;
	extern struct perf_buffer *pb;

	typedef void (* handle_cb_t)(void *, int, void *, __u32);

	int do_detach(int idx, const char *name);
	int bpf_init(handle_cb_t cb, int filt);
#ifdef __cplusplus
}
#endif

#endif
