#include "networkclient.h"

#include "../shared/protocol.h"

#include <QAbstractSocket>
#include <QJsonArray>
#include <QJsonObject>
#include <QTcpSocket>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent),
      m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit networkError(m_socket->errorString());
    });
}

bool NetworkClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(3000);
}

void NetworkClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void NetworkClient::sendRegister(const QString &username, const QString &password)
{
    m_socket->write(Protocol::packJson(Protocol::createRegisterRequest(username, password)));
}

void NetworkClient::sendLogin(const QString &username, const QString &password)
{
    m_socket->write(Protocol::packJson(Protocol::createLoginRequest(username, password)));
}

void NetworkClient::requestUserList()
{
    QJsonObject obj;
    obj["type"] = "userlist_request";
    obj["time"] = Protocol::currentTimeString();
    m_socket->write(Protocol::packJson(obj));
}

void NetworkClient::sendChat(const QString &from, const QString &to, const QString &message)
{
    m_socket->write(Protocol::packJson(Protocol::createChatMessage(from, to, message)));
}

void NetworkClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    const QList<QJsonObject> messages = Protocol::unpackMessages(m_buffer);
    for (const QJsonObject &obj : messages) {
        processMessage(obj);
    }
}

void NetworkClient::onConnected()
{
    emit connected();
}

void NetworkClient::onDisconnected()
{
    emit disconnected();
}

void NetworkClient::processMessage(const QJsonObject &obj)
{
    const QString type = obj.value("type").toString();
    const QString status = obj.value("status").toString();
    const QString message = obj.value("message").toString();

    if (type == "register_reply") {
        emit registerResult(status == "ok", message);
        return;
    }

    if (type == "login_reply") {
        emit loginResult(status == "ok", message);
        return;
    }

    if (type == "userlist") {
        QStringList users;
        const QJsonArray array = obj.value("users").toArray();
        for (const QJsonValue &value : array) {
            users.append(value.toString());
        }
        emit userListReceived(users);
        return;
    }

    if (type == "chat") {
        emit chatMessageReceived(obj.value("from").toString(),
                                 obj.value("to").toString(),
                                 obj.value("message").toString(),
                                 obj.value("time").toString());
        return;
    }

    if (type == "chat_reply") {
        emit sendMessageResult(status == "ok", message, obj.value("to").toString());
    }
}
