#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class MainWindow;
class NetworkClient;

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() override;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLoginResult(bool success, const QString &message);
    void onNetworkError(const QString &message);

private:
    bool ensureConnected();

    NetworkClient *m_networkClient;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QLabel *m_tipLabel;
    MainWindow *m_mainWindow;
};

#endif
