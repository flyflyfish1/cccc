#include "staticfileserver.h"

#include <QDir>
#include <QFile>
#include <QTcpSocket>
#include <QTextStream>

StaticFileServer::StaticFileServer(const QString &rootDir, QObject *parent)
    : QTcpServer(parent),
      m_rootDir(QDir(rootDir).absolutePath())
{
}

void StaticFileServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        socket->deleteLater();
        return;
    }

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        const QByteArray request = socket->readAll();
        const QList<QByteArray> lines = request.split('\n');
        if (lines.isEmpty()) {
            socket->disconnectFromHost();
            return;
        }

        const QByteArray requestLine = lines.first().trimmed();
        const QList<QByteArray> parts = requestLine.split(' ');
        if (parts.size() < 2 || parts[0] != "GET") {
            socket->write("HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n");
            socket->disconnectFromHost();
            return;
        }

        socket->write(buildHttpResponse(QString::fromUtf8(parts[1])));
        socket->disconnectFromHost();
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

QByteArray StaticFileServer::buildHttpResponse(const QString &requestPath) const
{
    const QString cleanPath = sanitizePath(requestPath);
    const QString fullPath = QDir(m_rootDir).absoluteFilePath(cleanPath);
    QFile file(fullPath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\nNot Found";
    }

    const QByteArray body = file.readAll();
    QByteArray header;
    QTextStream stream(&header);
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-Type: " << mimeTypeForPath(fullPath) << "\r\n";
    stream << "Content-Length: " << body.size() << "\r\n";
    stream << "Cache-Control: no-cache\r\n";
    stream << "Connection: close\r\n\r\n";
    stream.flush();

    return header + body;
}

QString StaticFileServer::mimeTypeForPath(const QString &path) const
{
    if (path.endsWith(".html")) {
        return "text/html; charset=utf-8";
    }
    if (path.endsWith(".css")) {
        return "text/css; charset=utf-8";
    }
    if (path.endsWith(".js")) {
        return "application/javascript; charset=utf-8";
    }
    if (path.endsWith(".json")) {
        return "application/json; charset=utf-8";
    }
    if (path.endsWith(".png")) {
        return "image/png";
    }
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) {
        return "image/jpeg";
    }
    if (path.endsWith(".svg")) {
        return "image/svg+xml";
    }
    return "text/plain; charset=utf-8";
}

QString StaticFileServer::sanitizePath(const QString &requestPath) const
{
    QString path = requestPath;
    const int queryIndex = path.indexOf('?');
    if (queryIndex >= 0) {
        path = path.left(queryIndex);
    }

    if (path.isEmpty() || path == "/") {
        return "index.html";
    }

    path.remove(0, path.startsWith('/') ? 1 : 0);
    if (path.contains("..")) {
        return "index.html";
    }
    return path;
}
