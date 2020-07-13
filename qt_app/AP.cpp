#include "AP.h"

AP::AP(QString ssid, QString bssid, int freq, QString bandwidth, QString chan, int rssi, QString security, QString country)
{
    SSID = ssid;
    BSSID = bssid;
    Frequency = freq;
    Bandwidth = bandwidth;
    Channel = chan;
    RSSI = rssi - 256;
    Security = security;
    Country = country;
    update_time = std::time(0);
}

void AP::UpdateAP(QString ssid, int freq, QString bandwidth, QString chan, int rssi, QString security, QString country)
{
    SSID = ssid;
    Frequency = freq;
    Bandwidth = bandwidth;
    Channel = chan;
    RSSI = rssi - 256;
    Security = security;
    Country = country;
    update_time = std::time(0);
}
