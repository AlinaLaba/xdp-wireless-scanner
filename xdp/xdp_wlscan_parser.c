#include <stdio.h>
#include <errno.h>
#include <net/if.h>
#include <ctype.h>
#include <stdlib.h> 
#include <string.h>

#include "xdp_wlscan_parser.h"

#define WLAN_CAPABILITY_PRIVACY	(1 << 4)
#define SSID_ID 0
#define RSN_ID 48
#define HT_INFO_ID 61
#define VHT_OPERATION_ID 192
#define VENDOR_ID 221
#define COUNTRY_ID 7
#define WPA2_FOUND_FLAG 0x01
#define IEEE802_FOUND_FLAG 0x02
#define OFFSET_CHECK_FLAG 0x03
#define MAX_CHANNEL_2_4 14
#define RSN_GROUP_CIPH_SUITE_LEN 4
#define RSN_VERSION_LEN 2
#define ID_LEN 2
#define RSN_PAIRWISE_CIPH_SUITE_LEN 4
#define RSN_COUNT_LEN 2
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define WPA "WPA"
#define WPA2 "WPA2"
#define IEEE802 "802.1X"
#define MAC_ADDR_LEN 18
#define SSID_MAX_LEN 256
#define CHAN_2_4_MIN 2412
#define CHAN_2_4_MAX 2484
#define CHAN_2_4_DIFF 1
#define CHAN_5_MIN 5170
#define CHAN_5_MAX 5825
#define CHAN_5_DIFF 34
#define IDX_RSSI 30 //1b or 30
#define IDX_FREQ 26 //2b
#define IDX_BSSID 70 //6b
#define IDX_TAGGED_PARAMS 90 //

static int freq2chan(int freq)
{
	int res;

	if (freq >= CHAN_2_4_MIN && freq <= CHAN_2_4_MAX)
	{
		res = (freq - CHAN_2_4_MIN) / 5 + 1;
		return res == 15 ? 14 : res;
	}

	if (freq >= CHAN_5_MIN && freq <= CHAN_5_MAX)
		return (freq - CHAN_5_MIN) / 5 + 34;

	return -1;
}

typedef struct wifi_scan_print_data_t {
	char mac_addr[MAC_ADDR_LEN];
	int freq;
	char country[4];
	__u8 signal;
	int wpa_found;
	uint8_t security_data;
	char ssid[SSID_MAX_LEN];
	int ht_chan_offset;
	int ht_primary_chan;
	int ht_secondary_chan;
	int vht_width;
	int vht_centr_chan_1;
	int vht_centr_chan_2;
	int primary_chan_1;
	int primary_chan_2;
	int secondary_chan_1;
	int secondary_chan_2;
} wifi_scan_print_data_t;

enum offset_t {
	NO_OFFSET,
	OFFSET_ABOVE,
	OFFSET_RESERVED,
	OFFSET_BELOW,
};

enum width2chan_t {
	CHAN_40MHZ = 4,
	CHAN_80MHZ = 8,
	CHAN_160MHZ = 16,
};

enum width_oui_t {
	CHAN_20_40MHZ_WIDTH,
	CHAN_80MHZ_WIDTH,
	CHAN_160MHZ_WIDTH,
	CHAN_80_80MHZ_WIDTH,
};

static unsigned char wpa_oui[] = { 0x00, 0x50, 0xf2, 0x01 };
static unsigned char akm_oui[] = { 0x00, 0x0f, 0xac };

static int parse_vendor(uint8_t *ie, int ielen)
{
	int i = 0;

	i += ID_LEN;

	if (!memcmp(ie + i, wpa_oui, ARRAY_SIZE(wpa_oui)))
		return 1;

	return 0;
}

static int parse_rsn(uint8_t *ie, int ielen)
{
	uint8_t counter, ret = 0, i = 0;

	i += ID_LEN;

	i += RSN_VERSION_LEN + RSN_GROUP_CIPH_SUITE_LEN;
	counter = ie[i];
	i += RSN_PAIRWISE_CIPH_SUITE_LEN * counter + RSN_COUNT_LEN;
	counter = ie[i];
	i += RSN_COUNT_LEN;

	while (counter > 0)
	{
		if (!memcmp(ie + i + 1, akm_oui, ARRAY_SIZE(akm_oui)))
			break;

		if (ie[i + 3] == 1)
			ret = WPA2_FOUND_FLAG | IEEE802_FOUND_FLAG;
		else if (ie[i + 3] == 2)
			ret |= WPA2_FOUND_FLAG;
		else if (ie[i + 3] == 3)
			ret |= IEEE802_FOUND_FLAG;

		counter--;
	}

	if (ret)
		return ret;

	return 0;
}

static int parse_ht_info(wifi_scan_print_data_t *data, uint8_t *ie, int ielen)
{
	int i = 0;

	i += ID_LEN;

	data->ht_primary_chan = ie[i];
	data->ht_chan_offset = ie[i + 1] & OFFSET_CHECK_FLAG;

	return 0;
}

static int parse_vht_operation(wifi_scan_print_data_t *data, uint8_t *ie,
	int ielen)
{
	int i = 0;

	i += ID_LEN;

	if (!ie[i] || !ie[i + 1])
		return -1;

	data->vht_width = ie[i];
	data->vht_centr_chan_1 = ie[i + 1];
	data->vht_centr_chan_2 = ie[i + 2];

	return 0;
}

static void fill_chan(wifi_scan_print_data_t *data)
{
	if (data->vht_centr_chan_1)
	{
		if (data->vht_width == CHAN_160MHZ_WIDTH)
		{
			data->primary_chan_1 = data->vht_centr_chan_1 - (CHAN_160MHZ - 2);
			data->secondary_chan_1 = data->vht_centr_chan_1 + (CHAN_160MHZ - 2);
			return;
		}

		if (data->vht_width == CHAN_80MHZ_WIDTH)
		{
			data->primary_chan_1 = data->vht_centr_chan_1 - (CHAN_80MHZ - 2);
			data->secondary_chan_1 = data->vht_centr_chan_1 + (CHAN_80MHZ - 2);
			return;
		}

		if (data->vht_width == CHAN_80_80MHZ_WIDTH)
		{
			data->primary_chan_1 = data->vht_centr_chan_1 - (CHAN_80MHZ - 2);
			data->secondary_chan_1 = data->vht_centr_chan_1 + (CHAN_80MHZ - 2);
			data->primary_chan_2 =  data->vht_centr_chan_2 - (CHAN_80MHZ - 2);
			data->secondary_chan_2 = data->vht_centr_chan_2 + (CHAN_80MHZ - 2);
			return;
		}
	}

	if (data->ht_chan_offset)
	{
		if (data->ht_chan_offset == OFFSET_BELOW)
		{
			data->primary_chan_1 = data->ht_primary_chan;
			data->secondary_chan_1 = data->ht_primary_chan - CHAN_40MHZ;
			return;
		}

		if (data->ht_chan_offset == OFFSET_ABOVE)
		{
			data->primary_chan_1 = data->ht_primary_chan;
			data->secondary_chan_1 = data->ht_primary_chan + CHAN_40MHZ;
			return;
		}
	}

	if (data->ht_primary_chan)
	{
		data->primary_chan_1 = data->ht_primary_chan;
		return;
	}

	data->primary_chan_1 = freq2chan(data->freq);
}

static ap_data_t prepare_data(wifi_scan_print_data_t *p_data)
{
	ap_data_t ap = {};

	ap.rssi = p_data->signal;

	if (p_data->vht_width == CHAN_80MHZ_WIDTH)
		sprintf(ap.bandwidth, "80");
	else if (p_data->vht_width == CHAN_160MHZ_WIDTH)
		sprintf(ap.bandwidth, "160");
	else if (p_data->vht_width == CHAN_80_80MHZ_WIDTH)
		sprintf(ap.bandwidth, "80+80");
	else if (p_data->ht_chan_offset == OFFSET_ABOVE || p_data->ht_chan_offset == OFFSET_BELOW)
		sprintf(ap.bandwidth, "40");
	else
		sprintf(ap.bandwidth, "20");

	sprintf(ap.bssid, "%s", p_data->mac_addr);
	sprintf(ap.country, "%s", p_data->country);
	ap.freq = p_data->freq;

	if (p_data->primary_chan_2)
		sprintf(ap.chan, "%d+%d %d+%d", p_data->primary_chan_1, p_data->secondary_chan_1, p_data->primary_chan_2, p_data->secondary_chan_2);
	else if (p_data->secondary_chan_1)
		sprintf(ap.chan, "%d+%d", p_data->primary_chan_1, p_data->secondary_chan_1);
	else
		sprintf(ap.chan, "%d", p_data->primary_chan_1);

	if (p_data->security_data == (WPA2_FOUND_FLAG | IEEE802_FOUND_FLAG))
	{
		if (!p_data->wpa_found)
			sprintf(ap.security, WPA2" "IEEE802);
		else
			sprintf(ap.security, WPA" "WPA2" "IEEE802);
	}
	else if (p_data->security_data == WPA2_FOUND_FLAG)
	{
		if (!p_data->wpa_found)
			sprintf(ap.security, WPA2);
		else
			sprintf(ap.security, WPA" "WPA2);
	}
	else if (p_data->security_data == IEEE802_FOUND_FLAG)
	{
		if (!p_data->wpa_found)
			sprintf(ap.security, IEEE802);
		else
			sprintf(ap.security, WPA" "IEEE802);
	}

	sprintf(ap.ssid, "%s", p_data->ssid);

	return ap;
}

ap_data_t parse_start(__u8 *frame, __u16 len)
{
	wifi_scan_print_data_t p_data = {};
	int privacy_bit = 1;

	p_data.freq = ((__u16 *)frame)[IDX_FREQ / 2];
	p_data.signal = frame[IDX_RSSI];

	sprintf(p_data.mac_addr, "%x:%x:%x:%x:%x:%x", frame[IDX_BSSID], frame[IDX_BSSID + 1],
		frame[IDX_BSSID + 2], frame[IDX_BSSID + 3], frame[IDX_BSSID + 4], frame[IDX_BSSID + 5]);

	int c, j, ssid_len;
	p_data.ssid[0] = '\0';
	
	__u16 i = IDX_TAGGED_PARAMS;

	while(i < len)
	{
		switch (frame[i])
		{
			case SSID_ID:
				ssid_len = frame[IDX_TAGGED_PARAMS + 1];

				for (c = IDX_TAGGED_PARAMS + 2, j = 0; c < IDX_TAGGED_PARAMS + ssid_len + 2; c++, j++)
					p_data.ssid[j] = frame[c];

				p_data.ssid[j] = '\0';
				i += frame[i + 1] + 2;
				break;
			case VENDOR_ID:
				if (privacy_bit)
					if (parse_vendor(frame + i, frame[i + 1]))
						p_data.wpa_found = 1;
				i += frame[i + 1] + 2;
				break;
			case RSN_ID:
				if (privacy_bit)
					p_data.security_data = parse_rsn(frame + i, frame[i + 1]);
				i += frame[i + 1] + 2;
				break;
			case VHT_OPERATION_ID:
				parse_vht_operation(&p_data, frame + i, frame[i + 1]);
				i += frame[i + 1] + 2;
				break;
			case HT_INFO_ID:
				parse_ht_info(&p_data, frame + i, frame[i + 1]);
				i += frame[i + 1] + 2;
				break;
			case COUNTRY_ID:
				p_data.country[0] = frame[i + 2];
				p_data.country[1] = frame[i + 3];
				p_data.country[2] = '\0';
				i += frame[i + 1] + 2;
				break;
			default:
				i += frame[i + 1] + 2;
		}
	}

	fill_chan(&p_data);

	return prepare_data(&p_data);
}
