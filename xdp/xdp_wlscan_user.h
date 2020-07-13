#ifndef _XDP_WLSCAN_USER_H_
#define _XDP_WLSCAN_USER_H_

#ifdef __cplusplus
extern "C"
{
#endif
	int wlscan_start(char *intf);
	void wlscan_end(int sig);
	int wlscan_intf_valid(const char *intf);
	void wlscan_set_filter(int filt);
#ifdef __cplusplus
}
#endif

#endif
