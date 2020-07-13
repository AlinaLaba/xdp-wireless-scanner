#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <net/if.h>

#ifdef __cplusplus
extern "C"
{
#endif
	#include <bpf/libbpf.h>
	#include <bpf/bpf.h>
	#include <libbpf.h>
	#include <linux/bpf.h>
	#include <linux/perf_event.h>

	#include "perf-sys.h"
	#include "xdp_wlscan_bpf_utils.h"
#ifdef __cplusplus
}
#endif

#define XDP_FLAGS_UPDATE_IF_NOEXIST     (1U << 0)
#define LOAD_FILE "/home/alina/xdp_wireless_scanner/xdp_wlscan_kern.o"

int if_idx;
char *if_name;
static __u32 prog_id;
struct perf_buffer *pb;

static int do_attach(int idx, int fd, const char *name)
{
	struct bpf_prog_info info = {};
	__u32 info_len = sizeof(info);
	int err;

	err = bpf_set_link_xdp_fd(idx, fd, XDP_FLAGS_UPDATE_IF_NOEXIST);
	if (err < 0) {
		printf("ERROR: failed to attach program to %s\n", name);
		return err;
	}

	err = bpf_obj_get_info_by_fd(fd, &info, &info_len);
	if (err) {
		printf("can't get prog info - %s\n", strerror(errno));
		return err;
	}
	prog_id = info.id;

	return err;
}

int do_detach(int idx, const char *name)
{
	__u32 curr_prog_id = 0;
	int err = 0;

	err = bpf_get_link_xdp_id(idx, &curr_prog_id, 0);
	if (err) {
		printf("bpf_get_link_xdp_id failed\n");
		return err;
	}
	if (prog_id == curr_prog_id) {
		err = bpf_set_link_xdp_fd(idx, -1, 0);
		if (err < 0)
			printf("ERROR: failed to detach prog from %s\n", name);
	} else if (!curr_prog_id) {
		printf("couldn't find a prog id on a %s\n", name);
	} else {
		printf("program on interface changed, not removing\n");
	}

	return err;
}

int bpf_init(handle_cb_t cb, int filter)
{
	struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
	struct bpf_prog_load_attr prog_load_attr =
	{
		.prog_type	= BPF_PROG_TYPE_XDP,
	};
	struct perf_buffer_opts pb_opts = {};
	struct bpf_object *obj;
	struct bpf_map *map /*, *map2*/;
	char filename[256];
	int prog_fd, map_fd, key = 1;

	if (setrlimit(RLIMIT_MEMLOCK, &r))
	{
		printf("setrlimit(RLIMIT_MEMLOCK, RLIM_INFINITY)\n");
		return -1;
	}

	snprintf(filename, sizeof(filename), LOAD_FILE);
	prog_load_attr.file = filename;

	if (bpf_prog_load_xattr(&prog_load_attr, &obj, &prog_fd))
	{
		printf("bpf_prog_load_xattr failed: %s", strerror(errno));
		return -1;
	}

	if (!prog_fd)
	{
		printf("load_bpf_file: %s\n", strerror(errno));
		return -1;
	}

	if (!(map = bpf_map__next(NULL, obj)))
	{
		printf("finding a map in obj file failed\n");
		return -1;
	}

	map_fd = bpf_map__fd(bpf_object__find_map_by_name(obj, "filter_map"));

	bpf_map_update_elem(map_fd, &key, &filter, BPF_ANY);

	if_name = MON_INTF;

	if (!(if_idx = if_nametoindex(MON_INTF)))
	{
		printf("Invalid ifname\n");
		return -1;
	}

	if (do_attach(if_idx, prog_fd, if_name))
	{
		printf("Cannot attach\n");
		return -1;
	}

	pb_opts.sample_cb = cb;
	pb = perf_buffer__new(bpf_map__fd(map), 8, &pb_opts);

	if (libbpf_get_error(pb))
	{
		printf("perf_buffer setup failed\n");
		return -1;
	}

	return 0;
}

