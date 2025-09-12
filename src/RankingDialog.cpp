#include "RankingDialog.h"
#include "ui_RankingDialog.h"
#include <QMessageBox>
#include <QPushButton>

RankingDialog::RankingDialog(NetworkClient *networkClient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RankingDialog)
    , network_client(networkClient)
    , login_dialog(nullptr)
    , level_ranking_dialog(nullptr)
    , time_ranking_dialog(nullptr)
    , step_ranking_dialog(nullptr)
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setModal(true);
    setWindowTitle("排行榜");
    
    // 创建子对话框
    login_dialog = new LoginDialog(network_client, this);
    level_ranking_dialog = new LevelRankingDialog(network_client, this);
    time_ranking_dialog = new TimeRankingDialog(network_client, this);
    step_ranking_dialog = new StepRankingDialog(network_client, this);
    
    // 连接信号
    connect(login_dialog, &LoginDialog::loginSuccessful, this, &RankingDialog::onLoginSuccessful);
    connect(network_client, &NetworkClient::logoutFinished, this, &RankingDialog::onLogoutFinished);
    
    // 初始化界面
    updateUserInfo();
}

RankingDialog::~RankingDialog()
{
    delete ui;
}

void RankingDialog::setCurrentUser(const UserInfo &user)
{
    current_user = user;
    updateUserInfo();
}

void RankingDialog::on_btnLevelRanking_clicked()
{
    checkLoginAndProceed([this]() {
        level_ranking_dialog->show();
        level_ranking_dialog->raise();
        level_ranking_dialog->activateWindow();
    });
}

void RankingDialog::on_btnTimeRanking_clicked()
{
    checkLoginAndProceed([this]() {
        time_ranking_dialog->show();
        time_ranking_dialog->raise();
        time_ranking_dialog->activateWindow();
    });
}

void RankingDialog::on_btnStepRanking_clicked()
{
    checkLoginAndProceed([this]() {
        step_ranking_dialog->show();
        step_ranking_dialog->raise();
        step_ranking_dialog->activateWindow();
    });
}

void RankingDialog::on_btnBack_clicked()
{
    emit backToMenu();
    accept();
}

void RankingDialog::onLoginSuccessful(const UserInfo &user)
{
    current_user = user;
    updateUserInfo();
}

void RankingDialog::onLogoutFinished()
{
    current_user = UserInfo();
    updateUserInfo();
}

void RankingDialog::updateUserInfo()
{
    if (current_user.isValid()) {
        ui->labelUserInfo->setText(QString("当前用户：%1 (%2)")
                                  .arg(current_user.nickname)
                                  .arg(current_user.username));
        ui->labelUserInfo->setStyleSheet("color: #4CAF50; font-weight: bold;");
    } else {
        ui->labelUserInfo->setText("未登录");
        ui->labelUserInfo->setStyleSheet("color: #666666;");
    }
}

void RankingDialog::checkLoginAndProceed(std::function<void()> action)
{
    if (!current_user.isValid()) {
        // 用户未登录，弹出登录对话框
        login_dialog->clearInputs();
        
        if (login_dialog->exec() == QDialog::Accepted && login_dialog->isLoggedIn()) {
            current_user = login_dialog->getCurrentUser();
            updateUserInfo();
            action();
        }
    } else {
        // 用户已登录，直接执行操作
        action();
    }
}