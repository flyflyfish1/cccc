#ifndef BRIDGESESSION_H
#define BRIDGESESSION_H

#include <QObject>

class QTcpSocket;
class QWebSocket;

class BridgeSession : public QObject
{
    Q_OBJECT
public:
    explicit BridgeSession(QWebSocket *webSocket,
                           const QString &serverHost,
                           quint16 serverPort,
                           QObject *parent = nullptr);

private slots:
    void onBrowserMessage(const QString &message);
    void onBrowserDisconnected();
    void onTcpConnected();
    void onTcpReadyRead();
    void onTcpDisconnected();
    void onTcpErrorOccurred();

private:
    void sendSystemMessage(const QString &status, const QString &message);
    void closeSession();

    QWebSocket *m_webSocket;
    QTcpSocket *m_tcpSocket;
    QByteArray m_buffer;
};

#endif
