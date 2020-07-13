#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#ifdef __cplusplus
extern "C"
{
#endif
	#include <libbpf.h>
	#include "perf-sys.h"
	#include "xdp_wlscan_user.h"
	#include "xdp_wlscan_probe_req.h"
	#include "xdp_wlscan_bpf_utils.h"
	#include "xdp_wlscan_parser.h"
	#include "xdp_wlscan_subscriber.h"
#ifdef __cplusplus
}
#endif

#define SAMPLE_SIZE 64

extern int if_idx;
extern char *if_name;
extern struct perf_buffer *pb;
static int filter = 0;

void wlscan_end(int sig)
{
	do_detach(if_idx, if_name);
	perf_buffer__free(pb);

	printf("Stopping.\n");

	if (system("sudo iw dev "MON_INTF" del"))
		exit(0);
	else
		exit(1);
}

static void handle_packet(void *ctx, int cpu, void *data, __u32 size)
{
	printf("handling packet\n");
	struct {
		__u16 cookie;
		__u16 pkt_len;
		__u8  pkt_data[SAMPLE_SIZE];
	} __packed *e = data;

	if (e->cookie != 0xdead) {
		printf("BUG cookie %x sized %d\n", e->cookie, size);
		return;
	}

	notify(parse_start(e->pkt_data, e->pkt_len));

	printf("Notified\n");
}

static int add_mon_intf(void)
{
	if (system("iw phy phy0 interface add "MON_INTF" type monitor") == -1)
	{
		printf("mon cmd failed!\n");
		return -1;
	}

	if (system("ifconfig "MON_INTF" up") == -1)
	{
		printf("mon0 up failed!\n");
		return -1;
	}

	return 0;
}

int wlscan_intf_valid(const char *intf)
{
	if (!if_nametoindex(intf))
		return 0;

	int sock = -1;
	struct iwreq pwrq = {};
	strncpy(pwrq.ifr_name, intf, IFNAMSIZ);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return 0;
	}

	if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1)
	{
		close(sock);
		return 1;
	}

	close(sock);
	return -1;
}

void wlscan_set_filter(int filt)
{
	filter = filt;
}

int wlscan_start(char *intf)
{
	signal(SIGINT, wlscan_end);
	signal(SIGTERM, wlscan_end);

	if (add_mon_intf()== -1)
		return -1;

	if (bpf_init(handle_packet, filter) == -1)
		return -1;

	printf("bpf initialised\n");

	for (;;) {
		send_probe_req(intf);
		printf("sent probe request\n");
		perf_buffer__poll(pb, 1000);
		printf("poll exited\n");
		sleep(1);
	}

	return 0;
}
