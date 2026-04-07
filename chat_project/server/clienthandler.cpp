#include "clienthandler.h"

#include "chatserver.h"
#include "dbmanager.h"
#include "logger.h"
#include "../shared/protocol.h"

#include <QMetaObject>
#include <QMutexLocker>
#include <QTcpSocket>

ClientHandler::ClientHandler(qintptr socketDescriptor, ChatServer *server, DBManager *dbManager, QObject *parent)
    : QThread(parent),
      m_socketDescriptor(socketDescriptor),
      m_server(server),
      m_dbManager(dbManager),
      m_socket(nullptr)
{
}

ClientHandler::~ClientHandler()
{
    quit();
    wait(1000);
}

QString ClientHandler::username() const
{
    return m_username;
}

void ClientHandler::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor)) {
        Logger::instance().write("Failed to set socket descriptor in client handler.");
        return;
    }

    {
        QMutexLocker locker(&m_sendMutex);
        m_socket = &socket;
    }

    connect(&socket, &QTcpSocket::readyRead, &socket, [this, &socket]() {
        m_buffer.append(socket.readAll());
        const QList<QJsonObject> messages = Protocol::unpackMessages(m_buffer);
        for (const QJsonObject &obj : messages) {
            processMessage(obj);
        }
    });

    connect(&socket, &QTcpSocket::disconnected, &socket, [this]() {
        Logger::instance().write(QString("Client disconnected. user=%1").arg(m_username));
        if (!m_username.isEmpty()) {
            m_server->logoutUser(m_username);
        }
        quit();
    });

    exec();

    {
        QMutexLocker locker(&m_sendMutex);
        m_socket = nullptr;
    }
}

void ClientHandler::sendJson(const QJsonObject &obj)
{
    QMutexLocker locker(&m_sendMutex);
    if (!m_socket) {
        return;
    }
    QTcpSocket *socket = m_socket;
    const QByteArray data = Protocol::packJson(obj);
    QMetaObject::invokeMethod(socket, [socket, data]() {
        socket->write(data);
        socket->flush();
    }, Qt::QueuedConnection);
}

void ClientHandler::processMessage(const QJsonObject &obj)
{
    const QString type = obj.value("type").toString();

    if (type == "register") {
        const QString username = obj.value("username").toString().trimmed();
        const QString password = obj.value("password").toString();
        if (username.isEmpty() || password.isEmpty()) {
            sendResponse("register_reply", "error", "Username or password cannot be empty.");
            return;
        }

        QString error;
        if (m_server->registerUser(username, password, &error)) {
            Logger::instance().write(QString("Register success: %1").arg(username));
            sendResponse("register_reply", "ok", "Register success.");
        } else {
            Logger::instance().write(QString("Register failed: %1, reason=%2").arg(username, error));
            sendResponse("register_reply", "error", QString("Register failed: %1").arg(error));
        }
        return;
    }

    if (type == "login") {
        const QString username = obj.value("username").toString().trimmed();
        const QString password = obj.value("password").toString();
        QString error;
        if (m_server->loginUser(username, password, this, &error)) {
            m_username = username;
            sendResponse("login_reply", "ok", "Login success.");
            sendJson(Protocol::createUserListMessage(m_server->onlineUsers()));
        } else {
            sendResponse("login_reply", "error", QString("Login failed: %1").arg(error));
        }
        return;
    }

    if (type == "chat") {
        const QString from = obj.value("from").toString();
        const QString to = obj.value("to").toString();
        const QString message = obj.value("message").toString();
        const QString time = obj.value("time").toString(Protocol::currentTimeString());
        if (from.isEmpty() || to.isEmpty() || message.trimmed().isEmpty()) {
            sendResponse("chat_reply", "error", "Invalid chat message.");
            return;
        }

        QString error;
        if (m_server->forwardChatMessage(from, to, message, time, &error)) {
            QJsonObject reply = Protocol::createSimpleResponse("chat_reply", "ok", "Message sent.");
            reply["to"] = to;
            sendJson(reply);
        } else {
            QJsonObject reply = Protocol::createSimpleResponse("chat_reply", "error", QString("Message delivery failed: %1").arg(error));
            reply["to"] = to;
            sendJson(reply);
        }
        return;
    }

    if (type == "history_request") {
        const QString user = obj.value("user").toString().trimmed();
        const QString peer = obj.value("peer").toString().trimmed();
        if (user.isEmpty() || peer.isEmpty()) {
            sendResponse("history_reply", "error", "Invalid history request.");
            return;
        }

        QString error;
        const QJsonArray records = m_server->loadConversation(user, peer, &error);
        QJsonObject reply;
        reply["type"] = "history_reply";
        reply["status"] = error.isEmpty() ? "ok" : "error";
        reply["user"] = user;
        reply["peer"] = peer;
        reply["records"] = records;
        reply["message"] = error.isEmpty() ? "History loaded." : error;
        reply["time"] = Protocol::currentTimeString();
        sendJson(reply);
        return;
    }

    if (type == "userlist_request") {
        sendJson(Protocol::createUserListMessage(m_server->onlineUsers()));
        return;
    }

    sendResponse("common_reply", "error", QString("Unknown type: %1").arg(type));
}

void ClientHandler::sendResponse(const QString &type, const QString &status, const QString &message)
{
    sendJson(Protocol::createSimpleResponse(type, status, message));
}
