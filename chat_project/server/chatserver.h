#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QMap>
#include <QMutex>
#include <QTcpServer>
#include <QStringList>

class ClientHandler;
class DBManager;

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    ~ChatServer() override;

    void setDbManager(DBManager *dbManager);
    bool registerUser(const QString &username, const QString &password, QString *errorMessage = nullptr);
    bool loginUser(const QString &username, const QString &password, ClientHandler *handler, QString *errorMessage = nullptr);
    void logoutUser(const QString &username);
    bool forwardChatMessage(const QString &from, const QString &to, const QString &message, const QString &time, QString *errorMessage = nullptr);
    QStringList onlineUsers() const;
    void sendUserListToAllClients();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    ClientHandler *handlerForUser(const QString &username) const;

    DBManager *m_dbManager;
    mutable QMutex m_mutex;
    QMap<QString, ClientHandler *> m_onlineUsers;
    QList<ClientHandler *> m_allHandlers;
};

#endif
