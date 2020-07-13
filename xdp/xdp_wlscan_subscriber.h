#ifndef _XDP_WLSCAN_SUBSCRIBER_H_
#define _XDP_WLSCAN_SUBSCRIBER_H_

#include "xdp_wlscan_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif
	typedef void (* reg_cb)(ap_data_t);
	extern reg_cb subscribed_cb;

	void wlscan_subscribe(reg_cb);
	void wlscan_unsubscribe(void);
	void notify(ap_data_t ap_data);
#ifdef __cplusplus
}
#endif

#endif
