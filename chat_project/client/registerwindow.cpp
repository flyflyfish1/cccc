#include "registerwindow.h"

#include "networkclient.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

RegisterWindow::RegisterWindow(NetworkClient *networkClient, QWidget *parent)
    : QDialog(parent),
      m_networkClient(networkClient)
{
    setWindowTitle("用户注册");
    resize(320, 180);

    m_usernameEdit = new QLineEdit(this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_registerButton = new QPushButton("注册", this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("用户名:", m_usernameEdit);
    formLayout->addRow("密码:", m_passwordEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_registerButton);

    connect(m_registerButton, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(m_networkClient, &NetworkClient::registerResult, this, &RegisterWindow::onRegisterResult);
}

void RegisterWindow::onRegisterClicked()
{
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空。");
        return;
    }
    m_networkClient->sendRegister(username, password);
}

void RegisterWindow::onRegisterResult(bool success, const QString &message)
{
    QMessageBox::information(this, success ? "注册成功" : "注册失败", message);
    if (success) {
        accept();
    }
}
