#ifndef AP_H
#define AP_H

#include <ctime>

#include <QString>

class AP
{
private:
    QString SSID;
    QString BSSID;
    uint Frequency;
    QString Bandwidth;
    QString Channel;
    int RSSI;
    QString Security;
    QString Country;
    time_t update_time;
public:
    AP(QString ssid, QString bssid, int freq, QString bandwidth, QString chan, int rssi, QString security, QString country);
    QString getSSID() { return SSID; }
    QString getBSSID() { return BSSID; }
    uint getFrequency() { return Frequency; }
    QString getBandwidth() { return Bandwidth; }
    QString getChannel() { return Channel; }
    int getRSSI() { return RSSI; }
    QString getSecurity() { return Security; }
    QString getCountry() { return Country; }
    time_t getUpdate_time() { return update_time; }
    void UpdateAP(QString ssid, int freq, QString bandwidth, QString chan, int rssi, QString security, QString country);
};

#endif // AP_H
