#include "bridgesession.h"

#include "../shared/protocol.h"

#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QWebSocket>

BridgeSession::BridgeSession(QWebSocket *webSocket,
                             const QString &serverHost,
                             quint16 serverPort,
                             QObject *parent)
    : QObject(parent),
      m_webSocket(webSocket),
      m_tcpSocket(new QTcpSocket(this))
{
    m_webSocket->setParent(this);

    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &BridgeSession::onBrowserMessage);
    connect(m_webSocket, &QWebSocket::disconnected, this, &BridgeSession::onBrowserDisconnected);

    connect(m_tcpSocket, &QTcpSocket::connected, this, &BridgeSession::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &BridgeSession::onTcpReadyRead);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &BridgeSession::onTcpDisconnected);
    connect(m_tcpSocket,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this,
            &BridgeSession::onTcpErrorOccurred);

    m_tcpSocket->connectToHost(serverHost, serverPort);
}

void BridgeSession::onBrowserMessage(const QString &message)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        sendSystemMessage("error", "Browser message is not valid JSON.");
        return;
    }

    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        sendSystemMessage("error", "Gateway is not connected to TCP chat server.");
        return;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("time")) {
        obj["time"] = Protocol::currentTimeString();
    }
    m_tcpSocket->write(Protocol::packJson(obj));
    m_tcpSocket->flush();
}

void BridgeSession::onBrowserDisconnected()
{
    closeSession();
}

void BridgeSession::onTcpConnected()
{
    sendSystemMessage("ok", "Gateway connected to TCP chat server.");
}

void BridgeSession::onTcpReadyRead()
{
    m_buffer.append(m_tcpSocket->readAll());
    const QList<QJsonObject> messages = Protocol::unpackMessages(m_buffer);
    for (const QJsonObject &obj : messages) {
        if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
            m_webSocket->sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
        }
    }
}

void BridgeSession::onTcpDisconnected()
{
    sendSystemMessage("error", "TCP chat server disconnected.");
    closeSession();
}

void BridgeSession::onTcpErrorOccurred()
{
    sendSystemMessage("error", m_tcpSocket->errorString());
}

void BridgeSession::sendSystemMessage(const QString &status, const QString &message)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QJsonObject obj = Protocol::createSimpleResponse("gateway_status", status, message);
    m_webSocket->sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

void BridgeSession::closeSession()
{
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
    }
    if (m_webSocket && m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
    }
    deleteLater();
}
