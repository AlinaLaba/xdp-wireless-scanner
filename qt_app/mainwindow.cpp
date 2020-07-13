#include <pthread.h>
#include <unistd.h>

#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UpdateHelper.h"
#include "xdp_wlscan_subscriber.h"
#include "xdp_wlscan_user.h"
#include "ap_dialog.h"

static bool scan_inited = false;
static int period = 1;
static char intf_name[255];

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->table->setColumnWidth(0, 195);
    ui->table->setColumnWidth(1, 145);
    ui->table->setColumnWidth(2, 100);
    ui->table->setColumnWidth(3, 103);
    ui->table->setColumnWidth(4, 145);
    ui->table->setColumnWidth(5, 100);
    ui->table->setColumnWidth(6, 145);
    ui->table->setColumnWidth(7, 100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (scan_inited)
        wlscan_end(1);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    UpdateHelper *helper = UpdateHelper::getInstance();

    for (int i = 0; i < helper->getAP_list().count(); i++)
    {
        if (!check_filter(helper->getAP_list()[i]))
            helper->getAP_list().removeAt(i);
    }

    ui->table->setRowCount(helper->getAP_list().count());
    ui->table->setColumnCount(8);

    for (int i = 0; i < helper->getAP_list().count(); i++)
    {
        QTableWidgetItem *item_ssid = new QTableWidgetItem(helper->getAP_list()[i].getSSID());
        QTableWidgetItem *item_bssid = new QTableWidgetItem(helper->getAP_list()[i].getBSSID());
        QTableWidgetItem *item_freq = new QTableWidgetItem(QString::number(helper->getAP_list()[i].getFrequency()).append(" MHz"));
        QTableWidgetItem *item_bandw = new QTableWidgetItem(helper->getAP_list()[i].getBandwidth().append(" MHz"));
        QTableWidgetItem *item_chan = new QTableWidgetItem(helper->getAP_list()[i].getChannel());
        QTableWidgetItem *item_rssi = new QTableWidgetItem(QString::number(helper->getAP_list()[i].getRSSI()).append(" dBm"));
        QTableWidgetItem *item_security = new QTableWidgetItem(helper->getAP_list()[i].getSecurity());
        QTableWidgetItem *item_country = new QTableWidgetItem(helper->getAP_list()[i].getCountry());

        item_freq->setTextAlignment(Qt::AlignCenter);
        item_bandw->setTextAlignment(Qt::AlignCenter);
        item_chan->setTextAlignment(Qt::AlignCenter);
        item_rssi->setTextAlignment(Qt::AlignCenter);
        item_security->setTextAlignment(Qt::AlignCenter);
        item_country->setTextAlignment(Qt::AlignCenter);

        ui->table->setItem(i, 0, item_ssid);
        ui->table->setItem(i, 1, item_bssid);
        ui->table->setItem(i, 2, item_freq);
        ui->table->setItem(i, 3, item_bandw);
        ui->table->setItem(i, 4, item_chan);
        ui->table->setItem(i, 5, item_rssi);
        ui->table->setItem(i, 6, item_security);
        ui->table->setItem(i, 7, item_country);
    }

    if (ui->comboBox->currentIndex() == 0)
        std::random_shuffle(helper->getAP_list().begin(), helper->getAP_list().end());
    else
        ui->table->sortByColumn(ui->comboBox->currentIndex() - 1, Qt::AscendingOrder);

}

void getNotification(ap_data_t data)
{
    UpdateHelper *helper = UpdateHelper::getInstance();

    helper->updateAPList(data);
}

void *start_thread(void *arg)
{
    wlscan_start(intf_name);
    return NULL;
}

void MainWindow::on_btn_start_clicked()
{
    if (ui->input_intf->text() == "")
    {
        QMessageBox::warning(this, tr("Error"), tr("Input interface"));
        return;
    }

    strcpy(intf_name, ui->input_intf->text().toStdString().c_str());

    if (!scan_inited)
    {
        pthread_t tid1;
        int intf_valid = wlscan_intf_valid(ui->input_intf->text().toStdString().c_str());

        if (!intf_valid)
        {
            QMessageBox::warning(this, tr("Error"), tr("Interface does not exist"));
            return;
        }
        else if (intf_valid == -1)
        {
            QMessageBox::warning(this, tr("Error"), tr("Interface is not wireless"));
            return;
        }

        wlscan_subscribe(getNotification);

        pthread_create(&tid1, 0, start_thread, NULL);
        pthread_detach(tid1);

        scan_inited = true;
    }

    timerId = startTimer(period * 1000);

    ui->btn_stop->setEnabled(true);
    ui->btn_start->setDisabled(true);
    ui->period->setDisabled(true);
    ui->input_intf->setDisabled(true);
    ui->actionStop_scan->setEnabled(true);
    ui->actionStart_scan->setDisabled(true);
    ui->actionSave_as->setDisabled(true);
}

void MainWindow::on_btn_stop_clicked()
{
    killTimer(timerId);

    ui->btn_stop->setDisabled(true);
    ui->btn_start->setEnabled(true);
    ui->period->setEnabled(true);
    ui->input_intf->setEnabled(true);
    ui->actionStop_scan->setDisabled(true);
    ui->actionStart_scan->setEnabled(true);
    ui->actionSave_as->setEnabled(true);
}

void MainWindow::on_table_cellClicked(int row, int column)
{
    UpdateHelper *helper = UpdateHelper::getInstance();
    AP *ap = &helper->getAP_list()[row];

    AP_dialog *dial = new AP_dialog(ap->getSSID(), ap->getBSSID(), ap->getFrequency(), ap->getBandwidth(), ap->getChannel(), ap->getRSSI(), ap->getSecurity(), ap->getCountry());
    dial->show();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"), tr("This application is developed to show wireless network access points configuration for "
                                                   "Linux kernel 5.4.0 and higher. The application GUI is created by Qt Creator 4.11.0, "
                                                   "based on Qt 5.12.8, GCC 9.3.0, 64 bit. \n"
                                                   "Network scanning is developed using eXpress Data Path technology using C and LLVM/Clang."));
}

void MainWindow::on_actionUser_guide_triggered()
{
    QMessageBox::information(this, tr("User guide"), tr("To start the network scanning, you should input name of your wireless interface "
                                                        "(you can see it using ifconfig) and press 'Start' button. You will see error "
                                                        "message if you try to use the wrong one.\n\n"
                                                        "You may want to use some of the other features:\n"
                                                        "1. Band filtering: simply choose 2.4 GHz or 5 GHz in the combobox.\n"
                                                        "2. Bandwidth filtering: choose desirable bandwidth in the combobox.\n"
                                                        "3. See window with AP configuration: press on any cell in the AP row.\n"
                                                        "4. Save results: stop the scanning and press 'Save results as ...' in menu File and choose the file.\n"));
}

void MainWindow::on_comboBox_currentIndexChanged(int idx)
{
    if (idx > 0)
        ui->table->sortByColumn(idx - 1, Qt::AscendingOrder);
}

void MainWindow::on_filter_band_activated(int index)
{
    UpdateHelper *helper = UpdateHelper::getInstance();

    if (index == 1 && ui->filter_bandwidth->currentIndex() > 2)
    {
        QMessageBox::warning(this, "Error", "Wrong filter settings");
        ui->filter_band->setCurrentIndex(0);
    }

    if (index == 0)
    {
        helper->filter &= ~FILTER_2_4GHZ;
        helper->filter &= ~FILTER_5_GHZ;
    }
    else if (index == 1)
    {
        helper->filter |= FILTER_2_4GHZ;
        helper->filter &= ~FILTER_5_GHZ;
    }
    else if (index == 2)
    {
        helper->filter |= FILTER_5_GHZ;
        helper->filter &= ~FILTER_2_4GHZ;
    }

    if (index == 0)
        wlscan_set_filter(0);
    else if (index == 1)
        wlscan_set_filter(FILTER_2_4GHZ);
    else if (index == 2)
        wlscan_set_filter(FILTER_5_GHZ);
}

void MainWindow::on_filter_bandwidth_activated(int index)
{
    UpdateHelper *helper = UpdateHelper::getInstance();

    if (index > 2 && ui->filter_band->currentIndex() == 1)
    {
        QMessageBox::warning(this, "Error", "Wrong filter settings");
        ui->filter_bandwidth->setCurrentIndex(0);
    }

    if (index == 0)
    {
        helper->filter &= ~FILTER_20_MHZ;
        helper->filter &= ~FILTER_40_MHZ;
        helper->filter &= ~FILTER_80_MHZ;
        helper->filter &= ~FILTER_160_MHZ;
        helper->filter &= ~FILTER_80_80_MHZ;
    }
    else if (index == 1)
    {
        helper->filter |= FILTER_20_MHZ;
        helper->filter &= ~FILTER_40_MHZ;
        helper->filter &= ~FILTER_80_MHZ;
        helper->filter &= ~FILTER_160_MHZ;
        helper->filter &= ~FILTER_80_80_MHZ;
    }
    else if (index == 2)
    {
        helper->filter |= FILTER_40_MHZ;
        helper->filter &= ~FILTER_20_MHZ;
        helper->filter &= ~FILTER_80_MHZ;
        helper->filter &= ~FILTER_160_MHZ;
        helper->filter &= ~FILTER_80_80_MHZ;
    }
    else if (index == 3)
    {
        helper->filter |= FILTER_80_MHZ;
        helper->filter &= ~FILTER_20_MHZ;
        helper->filter &= ~FILTER_40_MHZ;
        helper->filter &= ~FILTER_160_MHZ;
        helper->filter &= ~FILTER_80_80_MHZ;
    }
    else if (index == 4)
    {
        helper->filter |= FILTER_160_MHZ;
        helper->filter &= ~FILTER_20_MHZ;
        helper->filter &= ~FILTER_40_MHZ;
        helper->filter &= ~FILTER_80_MHZ;
        helper->filter &= ~FILTER_80_80_MHZ;
    }
    else if (index == 5)
    {
        helper->filter |= FILTER_80_80_MHZ;
        helper->filter &= ~FILTER_20_MHZ;
        helper->filter &= ~FILTER_40_MHZ;
        helper->filter &= ~FILTER_80_MHZ;
        helper->filter &= ~FILTER_160_MHZ;
    }
}

void MainWindow::on_actionStop_scan_triggered()
{
    killTimer(timerId);

    ui->btn_stop->setDisabled(true);
    ui->btn_start->setEnabled(true);
    ui->period->setEnabled(true);
    ui->input_intf->setEnabled(true);
    ui->actionStop_scan->setDisabled(true);
    ui->actionStart_scan->setEnabled(true);
    ui->actionSave_as->setEnabled(true);
}

void MainWindow::on_spinBox_valueChanged(const QString &arg1)
{}

void MainWindow::on_period_valueChanged(int arg1)
{
    period = arg1;
}

void MainWindow::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/home/", tr("Text (*.txt)"));

    FILE *f = fopen(fileName.toStdString().c_str(), "w");

    for (int i = 0; i < ui->table->rowCount(); i++)
    {
        for (int j = 0; j < ui->table->columnCount(); j++)
            fprintf(f, "%s;", ui->table->item(i, j)->text().toStdString().c_str());

        fprintf(f, "\n");
    }

    fclose(f);
}

void MainWindow::on_actionStart_scan_triggered()
{
    if (!scan_inited)
    {
        pthread_t tid1;

        wlscan_subscribe(getNotification);

        pthread_create(&tid1, 0, start_thread, NULL);
        pthread_detach(tid1);

        scan_inited = true;
    }

    timerId = startTimer(period * 1000);

    ui->btn_stop->setEnabled(true);
    ui->btn_start->setDisabled(true);
    ui->period->setDisabled(true);
    ui->input_intf->setDisabled(true);
    ui->actionStop_scan->setEnabled(true);
    ui->actionStart_scan->setDisabled(true);
    ui->actionSave_as->setDisabled(true);
}
