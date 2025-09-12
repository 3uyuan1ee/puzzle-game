#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "NetworkClient.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(NetworkClient *networkClient, QWidget *parent = nullptr);
    ~LoginDialog();

    UserInfo getCurrentUser() const;
    bool isLoggedIn() const;
    void clearInputs();

signals:
    void loginSuccessful(const UserInfo &user);

private slots:
    void on_btnLogin_clicked();
    void on_btnRegister_clicked();
    void on_btnCancel_clicked();
    void onLoginFinished(const NetworkResponse &response);
    void onRegisterFinished(const NetworkResponse &response);

private:
    Ui::LoginDialog *ui;
    NetworkClient *network_client;
    UserInfo current_user;
    bool is_logged_in;
    void showMessage(const QString &message, bool isError = false);
    void validateInputs();
};

#endif // LOGINDIALOG_H