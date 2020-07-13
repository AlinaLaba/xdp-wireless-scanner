#include "UpdateHelper.h"

#define VALID_TIME 10

UpdateHelper* UpdateHelper::instance = 0;

UpdateHelper* UpdateHelper::getInstance()
{
    if (!instance)
        instance = new UpdateHelper();
    return instance;
}

int check_filter(AP ap)
{
    UpdateHelper *helper = UpdateHelper::getInstance();

    if ((helper->filter & FILTER_5_GHZ && ap.getFrequency() < 3000) || (helper->filter & FILTER_2_4GHZ && ap.getFrequency() > 3000))
        return 0;

    if ((helper->filter & FILTER_20_MHZ && ap.getBandwidth() != "20") || (helper->filter & FILTER_40_MHZ && ap.getBandwidth() != "40") ||
            (helper->filter & FILTER_80_MHZ && ap.getBandwidth() != "80") || (helper->filter & FILTER_160_MHZ && ap.getBandwidth() != "160") ||
            (helper->filter & FILTER_80_80_MHZ && ap.getBandwidth() != "80+80"))
        return 0;

    return 1;
}

void UpdateHelper::updateAPList(ap_data_t ap_data)
{
    time_t now;
    AP ap(ap_data.ssid, ap_data.bssid, ap_data.freq, ap_data.bandwidth, ap_data.chan, ap_data.rssi, ap_data.security, ap_data.country);

    if (ap_data.ssid[0] == '\0' || ap_data.freq < 2000 || ap_data.bandwidth[0] == '\0' || ap_data.rssi - 256 >= 0)
        return;

    for (int i = 0; i < AP_list.count(); i++)
    {
        if (AP_list[i].getBSSID() == QString(ap_data.bssid))
        {
            if (!check_filter(AP_list[i]))
                AP_list.removeAt(i);
            else
                AP_list[i].UpdateAP(ap_data.ssid, ap_data.freq, ap_data.bandwidth, ap_data.chan, ap_data.rssi, ap_data.security, ap_data.country);

            return;
        }
    }

    now = std::time(0);

    for (int i = 0; i < AP_list.count(); i++)
        if (now - AP_list[i].getUpdate_time() > VALID_TIME)
            AP_list.removeAt(i);

    if (!check_filter(ap))
        return;

    AP_list.append(ap);
}

QList<AP>& UpdateHelper::getAP_list()
{
    return AP_list;
}
