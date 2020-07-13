#ifndef AP_DIALOG_H
#define AP_DIALOG_H

#include <QDialog>

namespace Ui {
class AP_dialog;
}

class AP_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit AP_dialog(QString ssid, QString bssid, int freq, QString bandw, QString chan, int rssi, QString security, QString country, QWidget *parent = nullptr);
    ~AP_dialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AP_dialog *ui;
};

#endif // AP_DIALOG_H
