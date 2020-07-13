#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "UpdateHelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void timerEvent(QTimerEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void on_btn_start_clicked();

    void on_btn_stop_clicked();

    void on_table_cellClicked(int row, int column);

    void on_actionAbout_triggered();

    void on_actionUser_guide_triggered();

    void on_comboBox_currentIndexChanged(int idx);

    void on_filter_band_activated(int index);

    void on_filter_bandwidth_activated(int index);

    void on_actionStop_scan_triggered();

    void on_spinBox_valueChanged(const QString &arg1);

    void on_period_valueChanged(int arg1);

    void on_actionSave_as_triggered();

    void on_actionStart_scan_triggered();

private:
    Ui::MainWindow *ui;
    int timerId;
};
#endif // MAINWINDOW_H
