#ifndef LEVELRANKINGDIALOG_H
#define LEVELRANKINGDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
class LevelRankingDialog;
}

class LevelRankingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LevelRankingDialog(NetworkClient *networkClient, QWidget *parent = nullptr);
    ~LevelRankingDialog();

private slots:
    void on_btnRefresh_clicked();
    void on_btnBack_clicked();
    void onLevelRankingsFinished(const NetworkResponse &response, const QList<LevelRankingInfo> &rankings);

private:
    Ui::LevelRankingDialog *ui;
    NetworkClient *network_client;

    void loadRankings();
    void updateTable(const QList<LevelRankingInfo> &rankings);
    void clearTable();
    void showStatus(const QString &message, bool isError = false);
};

#endif // LEVELRANKINGDIALOG_H