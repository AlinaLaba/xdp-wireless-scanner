#ifndef UPDATEHELPER_H
#define UPDATEHELPER_H

#include <QList>
#include "AP.h"
#include "xdp_wlscan_parser.h"

#define FILTER_2_4GHZ (1 << 0)
#define FILTER_5_GHZ (1 << 1)
#define FILTER_20_MHZ (1 << 2)
#define FILTER_40_MHZ (1 << 3)
#define FILTER_80_MHZ (1 << 4)
#define FILTER_160_MHZ (1 << 5)
#define FILTER_80_80_MHZ (1 << 6)

class UpdateHelper
{
private:
    static UpdateHelper *instance;
    QList<AP> AP_list;
    UpdateHelper(){}
    UpdateHelper(const UpdateHelper& root) = delete;
    UpdateHelper& operator=(const UpdateHelper&) = delete;
public:
    int filter;
    static UpdateHelper *getInstance();
    void updateAPList(ap_data_t ap);
    QList<AP>& getAP_list();
};

int check_filter(AP ap);

#endif // UPDATEHELPER_H
