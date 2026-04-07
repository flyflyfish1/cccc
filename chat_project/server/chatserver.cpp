#include "chatserver.h"

#include "clienthandler.h"
#include "dbmanager.h"
#include "logger.h"
#include "../shared/protocol.h"

#include <QMutexLocker>

ChatServer::ChatServer(QObject *parent)
    : QTcpServer(parent),
      m_dbManager(nullptr)
{
}

ChatServer::~ChatServer()
{
    QMutexLocker locker(&m_mutex);
    for (ClientHandler *handler : m_allHandlers) {
        if (handler) {
            handler->quit();
            handler->wait(1000);
            delete handler;
        }
    }
}

void ChatServer::setDbManager(DBManager *dbManager)
{
    m_dbManager = dbManager;
}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    ClientHandler *handler = new ClientHandler(socketDescriptor, this, m_dbManager);
    {
        QMutexLocker locker(&m_mutex);
        m_allHandlers.append(handler);
    }

    connect(handler, &ClientHandler::finished, this, [this, handler]() {
        QMutexLocker locker(&m_mutex);
        m_allHandlers.removeAll(handler);
    });

    handler->start();
    Logger::instance().write(QString("New client connected. descriptor=%1").arg(socketDescriptor));
}

bool ChatServer::registerUser(const QString &username, const QString &password, QString *errorMessage)
{
    if (!m_dbManager) {
        if (errorMessage) {
            *errorMessage = "Database manager is null.";
        }
        return false;
    }
    return m_dbManager->registerUser(username, password, errorMessage);
}

bool ChatServer::loginUser(const QString &username, const QString &password, ClientHandler *handler, QString *errorMessage)
{
    if (!m_dbManager) {
        if (errorMessage) {
            *errorMessage = "Database manager is null.";
        }
        return false;
    }

    if (!m_dbManager->validateUser(username, password, errorMessage)) {
        return false;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (m_onlineUsers.contains(username) && m_onlineUsers.value(username) != handler) {
            if (errorMessage) {
                *errorMessage = "User is already online.";
            }
            return false;
        }
        m_onlineUsers[username] = handler;
    }

    Logger::instance().write(QString("User login success: %1").arg(username));
    sendUserListToAllClients();
    return true;
}

void ChatServer::logoutUser(const QString &username)
{
    if (username.isEmpty()) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_onlineUsers.remove(username);
    }

    Logger::instance().write(QString("User logout: %1").arg(username));
    sendUserListToAllClients();
}

bool ChatServer::forwardChatMessage(const QString &from, const QString &to, const QString &message, const QString &time, QString *errorMessage)
{
    if (!m_dbManager) {
        if (errorMessage) {
            *errorMessage = "Database manager is null.";
        }
        return false;
    }

    if (!m_dbManager->saveMessage(from, to, message, time, errorMessage)) {
        return false;
    }

    ClientHandler *target = handlerForUser(to);
    if (!target) {
        if (errorMessage) {
            *errorMessage = QString("Target user [%1] is offline.").arg(to);
        }
        Logger::instance().write(QString("Message saved but delivery failed: %1 -> %2").arg(from, to));
        return false;
    }

    QJsonObject chatObj = Protocol::createChatMessage(from, to, message);
    chatObj["time"] = time;
    target->sendJson(chatObj);
    Logger::instance().write(QString("Message forwarded: %1 -> %2").arg(from, to));
    return true;
}

QJsonArray ChatServer::loadConversation(const QString &userA, const QString &userB, QString *errorMessage)
{
    if (!m_dbManager) {
        if (errorMessage) {
            *errorMessage = "Database manager is null.";
        }
        return QJsonArray();
    }
    return m_dbManager->loadConversation(userA, userB, errorMessage);
}

QStringList ChatServer::onlineUsers() const
{
    QMutexLocker locker(&m_mutex);
    return m_onlineUsers.keys();
}

void ChatServer::sendUserListToAllClients()
{
    const QStringList users = onlineUsers();
    const QJsonObject obj = Protocol::createUserListMessage(users);

    QMutexLocker locker(&m_mutex);
    for (auto it = m_onlineUsers.cbegin(); it != m_onlineUsers.cend(); ++it) {
        if (it.value()) {
            it.value()->sendJson(obj);
        }
    }
}

ClientHandler *ChatServer::handlerForUser(const QString &username) const
{
    QMutexLocker locker(&m_mutex);
    return m_onlineUsers.value(username, nullptr);
}
