#include "loginwindow.h"

#include "filehelper.h"
#include "mainwindow.h"
#include "networkclient.h"
#include "registerwindow.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
      m_networkClient(new NetworkClient(this)),
      m_mainWindow(nullptr)
{
    setWindowTitle("聊天系统登录");
    resize(360, 220);

    m_usernameEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_loginButton = new QPushButton("登录", this);
    m_registerButton = new QPushButton("注册", this);
    m_tipLabel = new QLabel("默认读取 config/server.conf", this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("用户名:", m_usernameEdit);
    formLayout->addRow("密码:", m_passwordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tipLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(m_networkClient, &NetworkClient::loginResult, this, &LoginWindow::onLoginResult);
    connect(m_networkClient, &NetworkClient::networkError, this, &LoginWindow::onNetworkError);

    FileHelper::ensureDirectory(FileHelper::projectRootPath() + "/history");
}

LoginWindow::~LoginWindow()
{
}

bool LoginWindow::ensureConnected()
{
    QString host;
    quint16 port = 8888;
    FileHelper::readServerConfig(&host, &port);
    if (m_networkClient->connectToServer(host, port)) {
        return true;
    }
    QMessageBox::warning(this, "连接失败", QString("无法连接服务器 %1:%2").arg(host).arg(port));
    return false;
}

void LoginWindow::onLoginClicked()
{
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空。");
        return;
    }

    if (!ensureConnected()) {
        return;
    }
    m_networkClient->sendLogin(username, password);
}

void LoginWindow::onRegisterClicked()
{
    if (!ensureConnected()) {
        return;
    }
    RegisterWindow dialog(m_networkClient, this);
    dialog.exec();
}

void LoginWindow::onLoginResult(bool success, const QString &message)
{
    if (!success) {
        QMessageBox::warning(this, "登录失败", message);
        return;
    }

    QMessageBox::information(this, "登录成功", message);
    const QString currentUser = m_usernameEdit->text().trimmed();
    if (!m_mainWindow) {
        m_mainWindow = new MainWindow(currentUser, m_networkClient);
    }
    m_mainWindow->show();
    hide();
}

void LoginWindow::onNetworkError(const QString &message)
{
    m_tipLabel->setText(QString("网络状态: %1").arg(message));
}
