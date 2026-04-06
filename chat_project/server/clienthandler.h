#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QJsonObject>
#include <QMutex>
#include <QThread>

class QTcpSocket;
class ChatServer;
class DBManager;

class ClientHandler : public QThread
{
    Q_OBJECT
public:
    explicit ClientHandler(qintptr socketDescriptor, ChatServer *server, DBManager *dbManager, QObject *parent = nullptr);
    ~ClientHandler() override;

    void sendJson(const QJsonObject &obj);
    QString username() const;

protected:
    void run() override;

private:
    void processMessage(const QJsonObject &obj);
    void sendResponse(const QString &type, const QString &status, const QString &message);

    qintptr m_socketDescriptor;
    ChatServer *m_server;
    DBManager *m_dbManager;
    QTcpSocket *m_socket;
    QByteArray m_buffer;
    QString m_username;
    mutable QMutex m_sendMutex;
};

#endif
