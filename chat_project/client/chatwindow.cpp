#include "chatwindow.h"

#include "filehelper.h"
#include "networkclient.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ChatWindow::ChatWindow(const QString &currentUser, const QString &peerUser, NetworkClient *networkClient, QWidget *parent)
    : QWidget(parent),
      m_currentUser(currentUser),
      m_peerUser(peerUser),
      m_networkClient(networkClient)
{
    setWindowTitle(QString("与 %1 聊天").arg(peerUser));
    resize(480, 360);

    m_chatView = new QTextEdit(this);
    m_chatView->setReadOnly(true);
    m_inputEdit = new QLineEdit(this);
    m_sendButton = new QPushButton("发送", this);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chatView);
    mainLayout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatWindow::onSendClicked);
    connect(m_networkClient, &NetworkClient::sendMessageResult, this, &ChatWindow::onSendResult);
}

QString ChatWindow::peerUser() const
{
    return m_peerUser;
}

void ChatWindow::appendMessage(const QString &sender, const QString &message, const QString &time)
{
    const QString line = formatLine(sender, message, time);
    m_chatView->append(line);
    FileHelper::saveLocalHistory(m_currentUser, m_peerUser, line);
}

void ChatWindow::onSendClicked()
{
    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    m_networkClient->sendChat(m_currentUser, m_peerUser, text);
    appendMessage(m_currentUser, text, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    m_inputEdit->clear();
}

void ChatWindow::onSendResult(bool success, const QString &message, const QString &toUser)
{
    if (toUser != m_peerUser) {
        return;
    }
    if (!success) {
        QMessageBox::warning(this, "发送失败", message);
    }
}

QString ChatWindow::formatLine(const QString &sender, const QString &message, const QString &time) const
{
    return QString("[%1] %2: %3").arg(time, sender, message);
}
