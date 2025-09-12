#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QPushButton>

LoginDialog::LoginDialog(NetworkClient *networkClient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , network_client(networkClient)
    , is_logged_in(false)
{
    ui->setupUi(this);
    
    // 连接网络信号
    connect(network_client, &NetworkClient::loginFinished, this, &LoginDialog::onLoginFinished);
    connect(network_client, &NetworkClient::registerFinished, this, &LoginDialog::onRegisterFinished);
    
    // 设置窗口属性
    setModal(true);
    setWindowTitle("用户登录");
    
    // 初始化界面
    clearInputs();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

UserInfo LoginDialog::getCurrentUser() const
{
    return current_user;
}

bool LoginDialog::isLoggedIn() const
{
    return is_logged_in;
}

void LoginDialog::on_btnLogin_clicked()
{
    QString username = ui->lineEditUsername->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();
    
    if (username.isEmpty() || password.isEmpty()) {
        showMessage("请输入用户名和密码", true);
        return;
    }
    
    if (!network_client->isConnected()) {
        showMessage("网络连接失败，请检查网络设置", true);
        return;
    }
    
    ui->btnLogin->setEnabled(false);
    ui->btnRegister->setEnabled(false);
    showMessage("正在登录...");
    
    network_client->loginUser(username, password);
}

void LoginDialog::on_btnRegister_clicked()
{
    QString username = ui->lineEditUsername->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();
    QString nickname = ui->lineEditNickname->text().trimmed();
    
    if (username.isEmpty() || password.isEmpty() || nickname.isEmpty()) {
        showMessage("请填写完整信息", true);
        return;
    }
    
    if (!network_client->isConnected()) {
        showMessage("网络连接失败，请检查网络设置", true);
        return;
    }
    
    ui->btnLogin->setEnabled(false);
    ui->btnRegister->setEnabled(false);
    showMessage("正在注册...");
    
    network_client->registerUser(username, password, nickname);
}

void LoginDialog::on_btnCancel_clicked()
{
    reject();
}

void LoginDialog::onLoginFinished(const NetworkResponse &response)
{
    ui->btnLogin->setEnabled(true);
    ui->btnRegister->setEnabled(true);
    
    if (response.success) {
        current_user = network_client->getCurrentUser();
        is_logged_in = true;
        showMessage("登录成功！");
        
        // 延迟关闭对话框，让用户看到成功消息
        QTimer::singleShot(1000, this, [this]() {
            accept();
            emit loginSuccessful(current_user);
        });
    } else {
        showMessage(response.message, true);
    }
}

void LoginDialog::onRegisterFinished(const NetworkResponse &response)
{
    ui->btnLogin->setEnabled(true);
    ui->btnRegister->setEnabled(true);
    
    if (response.success) {
        showMessage("注册成功！请登录");
        ui->lineEditPassword->clear();
        ui->lineEditNickname->clear();
    } else {
        showMessage(response.message, true);
    }
}

void LoginDialog::clearInputs()
{
    ui->lineEditUsername->clear();
    ui->lineEditPassword->clear();
    ui->lineEditNickname->clear();
    ui->labelMessage->clear();
    ui->btnLogin->setEnabled(true);
    ui->btnRegister->setEnabled(true);
}

void LoginDialog::showMessage(const QString &message, bool isError)
{
    ui->labelMessage->setText(message);
    if (isError) {
        ui->labelMessage->setStyleSheet("color: red; font-weight: bold;");
    } else {
        ui->labelMessage->setStyleSheet("color: green; font-weight: bold;");
    }
}

void LoginDialog::validateInputs()
{
    QString username = ui->lineEditUsername->text().trimmed();
    QString password = ui->lineEditPassword->text().trimmed();
    QString nickname = ui->lineEditNickname->text().trimmed();
    
    ui->btnLogin->setEnabled(!username.isEmpty() && !password.isEmpty());
    ui->btnRegister->setEnabled(!username.isEmpty() && !password.isEmpty() && !nickname.isEmpty());
}