#ifndef RANKINGDIALOG_H
#define RANKINGDIALOG_H

#include <QDialog>
#include "NetworkClient.h"
#include "LoginDialog.h"
#include "LevelRankingDialog.h"
#include "TimeRankingDialog.h"
#include "StepRankingDialog.h"

namespace Ui {
class RankingDialog;
}

class RankingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RankingDialog(NetworkClient *networkClient, QWidget *parent = nullptr);
    ~RankingDialog();

    void setCurrentUser(const UserInfo &user);

signals:
    void backToMenu();

private slots:
    void on_btnLevelRanking_clicked();
    void on_btnTimeRanking_clicked();
    void on_btnStepRanking_clicked();
    void on_btnBack_clicked();
    void onLoginSuccessful(const UserInfo &user);
    void onLogoutFinished();

private:
    Ui::RankingDialog *ui;
    NetworkClient *network_client;
    UserInfo current_user;
    LoginDialog *login_dialog;
    LevelRankingDialog *level_ranking_dialog;
    TimeRankingDialog *time_ranking_dialog;
    StepRankingDialog *step_ranking_dialog;

    void updateUserInfo();
    void checkLoginAndProceed(std::function<void()> action);
};

#endif // RANKINGDIALOG_H