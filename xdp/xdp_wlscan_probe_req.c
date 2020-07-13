#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <net/if.h>

#include <libnl3/netlink/genl/genl.h>
#include <linux/nl80211.h>
#include <netlink/genl/ctrl.h>

#include "xdp_wlscan_probe_req.h"

#define DRIVER "nl80211"
#define GRP_NAME "scan"

typedef struct wifi_scan_data_t {
	int if_index;
	int driver_id;
	struct nl_sock *socket;
} wifi_scan_data_t;

static int send_start(wifi_scan_data_t *scan_data)
{
	int ret = -1, group_id;
	struct nl_msg *msg;
	struct nl_cb *cb;
	struct nl_msg *ssids_to_scan;

	if ((group_id = genl_ctrl_resolve_grp(scan_data->socket, DRIVER, GRP_NAME)) < 0) {
		fprintf(stderr, "family group resolving failed: %d", group_id);
		return -1;
	}

	if ((ret = nl_socket_add_membership(scan_data->socket, group_id))) {
		fprintf(stderr, "group joining failed: %d", ret);
		return -1;
	}

	if (!(msg = nlmsg_alloc())) {
		fprintf(stderr, "failed to allocate memory\n");
		nl_socket_drop_membership(scan_data->socket, group_id);
		return -1;
	}

	if (!(ssids_to_scan = nlmsg_alloc())) {
		fprintf(stderr, "failed to allocate memory\n");
		nlmsg_free(msg);
		nl_socket_drop_membership(scan_data->socket, group_id);
		return -1;
	}

	if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, scan_data->driver_id,
			0, 0, NL80211_CMD_TRIGGER_SCAN, 0)) {
		fprintf(stderr, "failed to add headers\n");
		goto exit;
	}

	if ((ret = nla_put_u32(msg, NL80211_ATTR_IFINDEX,
			scan_data->if_index))) {
		fprintf(stderr, "failed to add attribute\n");
		goto exit;
	}

	if ((ret = nla_put(ssids_to_scan, 1, 0, ""))) {
		fprintf(stderr, "failed to add attribute\n");
		goto exit;
	}

	if ((ret = nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS,
			ssids_to_scan))) {
		fprintf(stderr, "failed to add nested attribute\n");
		goto exit;
	}

	if (!(cb = nl_cb_alloc(NL_CB_DEFAULT))) {
		fprintf(stderr, "failed to allocate memory\n");
		goto exit;
	}

	if ((ret = nl_send_auto(scan_data->socket, msg)) < 0) {
		fprintf(stderr, "failed to send message\n");
		nl_cb_put(cb);
		goto exit;
	}

	ret = 0;
	nl_cb_put(cb);

exit:
	nlmsg_free(ssids_to_scan);
	nlmsg_free(msg);
	nl_socket_drop_membership(scan_data->socket, group_id);
	return ret;
}

int send_probe_req(char *interface)
{
	wifi_scan_data_t scan_data = {};

	if (!(scan_data.if_index = if_nametoindex(interface))) {
		fprintf(stderr, "if_nametoindex failed: %s %s\n", strerror(errno), interface);
		return -1;
	}

	if (!(scan_data.socket = nl_socket_alloc())) {
		fprintf(stderr, "socket allocation failed\n");
		return -1;
	}

	if (genl_connect(scan_data.socket)) {
		fprintf(stderr, "socket binding failed\n");
		goto error;
	}

	if ((scan_data.driver_id = genl_ctrl_resolve(scan_data.socket, DRIVER)) < 0) {
		fprintf(stderr, "name resolving failed: %d\n", scan_data.driver_id);
		goto error;
	}

	if (send_start(&scan_data))
		goto error;

	return 0;

error:
	nl_socket_free(scan_data.socket);
	return -1;
}
