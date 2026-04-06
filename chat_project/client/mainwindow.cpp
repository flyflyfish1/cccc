#include "mainwindow.h"

#include "chatwindow.h"
#include "networkclient.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(const QString &currentUser, NetworkClient *networkClient, QWidget *parent)
    : QMainWindow(parent),
      m_currentUser(currentUser),
      m_networkClient(networkClient)
{
    setWindowTitle(QString("聊天系统 - 当前用户: %1").arg(currentUser));
    resize(360, 420);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    m_statusLabel = new QLabel("在线用户列表", this);
    m_userListWidget = new QListWidget(this);
    m_refreshButton = new QPushButton("刷新在线用户", this);

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_userListWidget);
    layout->addWidget(m_refreshButton);

    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_userListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onUserDoubleClicked);
    connect(m_networkClient, &NetworkClient::userListReceived, this, &MainWindow::onUserListReceived);
    connect(m_networkClient, &NetworkClient::chatMessageReceived, this, &MainWindow::onChatMessageReceived);

    m_networkClient->requestUserList();
}

void MainWindow::onRefreshClicked()
{
    m_networkClient->requestUserList();
}

void MainWindow::onUserListReceived(const QStringList &users)
{
    m_userListWidget->clear();
    for (const QString &user : users) {
        if (user != m_currentUser) {
            m_userListWidget->addItem(user);
        }
    }
    m_statusLabel->setText(QString("在线用户数量: %1").arg(m_userListWidget->count()));
}

void MainWindow::onUserDoubleClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    ChatWindow *window = openChatWindow(item->text());
    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::onChatMessageReceived(const QString &from, const QString &to, const QString &message, const QString &time)
{
    const QString peerUser = (from == m_currentUser) ? to : from;
    ChatWindow *window = openChatWindow(peerUser);
    window->appendMessage(from, message, time);
    if (!window->isVisible()) {
        window->show();
    }
}

ChatWindow *MainWindow::openChatWindow(const QString &peerUser)
{
    if (m_chatWindows.contains(peerUser) && m_chatWindows.value(peerUser)) {
        return m_chatWindows.value(peerUser);
    }

    ChatWindow *window = new ChatWindow(m_currentUser, peerUser, m_networkClient);
    m_chatWindows[peerUser] = window;
    connect(window, &QObject::destroyed, this, [this, peerUser]() {
        m_chatWindows.remove(peerUser);
    });
    return window;
}
