#ifndef STEPRANKINGDIALOG_H
#define STEPRANKINGDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
class StepRankingDialog;
}

class StepRankingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StepRankingDialog(NetworkClient *networkClient, QWidget *parent = nullptr);
    ~StepRankingDialog();

private slots:
    void on_btnRefresh_clicked();
    void on_btnBack_clicked();
    void onStepRankingsFinished(const NetworkResponse &response, const QList<StepRankingInfo> &rankings);

private:
    Ui::StepRankingDialog *ui;
    NetworkClient *network_client;

    void loadRankings();
    void updateTable(const QList<StepRankingInfo> &rankings);
    void clearTable();
    void showStatus(const QString &message, bool isError = false);
};

#endif // STEPRANKINGDIALOG_H