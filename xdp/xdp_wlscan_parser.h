#ifndef _XDP_WLSCAN_PARSER_H_
#define _XDP_WLSCAN_PARSER_H_

#include <libnl3/netlink/genl/genl.h>
#include <linux/nl80211.h>
#include <netlink/genl/ctrl.h>

#define MAC_ADDR_LEN 18
#define COUNTRY_LEN 4
#define SECURITY_LEN 20
#define SSID_MAX_LEN 256
#define CHAN_MAX_LEN 20
#define BANDWIDTH_MAX_LEN 6

#ifdef __cplusplus
extern "C"
{
#endif
	typedef struct ap_data_t {
		char ssid[SSID_MAX_LEN];
		char bssid[MAC_ADDR_LEN];
		int freq;
		char bandwidth[BANDWIDTH_MAX_LEN];
		char chan[CHAN_MAX_LEN];
		int rssi;
		char security[SECURITY_LEN];
		char country[COUNTRY_LEN];
	} ap_data_t;

	ap_data_t parse_start(__u8 *frame, __u16 len);
#ifdef __cplusplus
}
#endif

#endif
