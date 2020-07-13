#include "xdp_wlscan_subscriber.h"

reg_cb subscribed_cb;

void wlscan_subscribe(reg_cb cb)
{
	subscribed_cb = cb;
}

void wlscan_unsubscribe(void)
{
	subscribed_cb = NULL;
}

void notify(ap_data_t ap_data)
{
	if (subscribed_cb)
		subscribed_cb(ap_data);
}
