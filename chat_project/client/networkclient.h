#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QJsonObject>
#include <QObject>

class QTcpSocket;

class NetworkClient : public QObject
{
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent = nullptr);

    bool connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();

    void sendRegister(const QString &username, const QString &password);
    void sendLogin(const QString &username, const QString &password);
    void requestUserList();
    void sendChat(const QString &from, const QString &to, const QString &message);

signals:
    void connected();
    void disconnected();
    void registerResult(bool success, const QString &message);
    void loginResult(bool success, const QString &message);
    void userListReceived(const QStringList &users);
    void chatMessageReceived(const QString &from, const QString &to, const QString &message, const QString &time);
    void sendMessageResult(bool success, const QString &message, const QString &toUser);
    void networkError(const QString &message);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    void processMessage(const QJsonObject &obj);

    QTcpSocket *m_socket;
    QByteArray m_buffer;
};

#endif
