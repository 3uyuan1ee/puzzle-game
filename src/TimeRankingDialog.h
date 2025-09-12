#ifndef TIMERANKINGDIALOG_H
#define TIMERANKINGDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
class TimeRankingDialog;
}

class TimeRankingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeRankingDialog(NetworkClient *networkClient, QWidget *parent = nullptr);
    ~TimeRankingDialog();

private slots:
    void on_btnRefresh_clicked();
    void on_btnBack_clicked();
    void onTimeRankingsFinished(const NetworkResponse &response, const QList<TimeRankingInfo> &rankings);

private:
    Ui::TimeRankingDialog *ui;
    NetworkClient *network_client;

    void loadRankings();
    void updateTable(const QList<TimeRankingInfo> &rankings);
    void clearTable();
    void showStatus(const QString &message, bool isError = false);
    QString formatTime(int seconds) const;
};

#endif // TIMERANKINGDIALOG_H