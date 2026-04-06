#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class NetworkClient;

class RegisterWindow : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterWindow(NetworkClient *networkClient, QWidget *parent = nullptr);

private slots:
    void onRegisterClicked();
    void onRegisterResult(bool success, const QString &message);

private:
    NetworkClient *m_networkClient;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_registerButton;
};

#endif
