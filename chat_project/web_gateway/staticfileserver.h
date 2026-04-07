#ifndef STATICFILESERVER_H
#define STATICFILESERVER_H

#include <QTcpServer>

class StaticFileServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit StaticFileServer(const QString &rootDir, QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QByteArray buildHttpResponse(const QString &requestPath) const;
    QString mimeTypeForPath(const QString &path) const;
    QString sanitizePath(const QString &requestPath) const;

    QString m_rootDir;
};

#endif
