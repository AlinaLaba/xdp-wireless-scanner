#include "ap_dialog.h"
#include "ui_ap_dialog.h"

AP_dialog::AP_dialog(QString ssid, QString bssid, int freq, QString bandw, QString chan, int rssi, QString security, QString country, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AP_dialog)
{
    ui->setupUi(this);

    ui->label_ssid->setText(ssid);
    ui->label_bssid->setText(bssid);
    ui->label_freq->setText(QString::number(freq).append(" MHz"));
    ui->label_bandw->setText(bandw.append(" MHz"));
    ui->label_chan->setText(chan);
    ui->label_rssi->setText(QString::number(rssi).append(" dBm"));
    ui->label_security->setText(security);
    ui->label_country->setText(country);
}

AP_dialog::~AP_dialog()
{
    delete ui;
}

void AP_dialog::on_pushButton_clicked()
{
    this->close();
}
