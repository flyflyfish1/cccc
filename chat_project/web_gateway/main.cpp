#include "bridgesession.h"
#include "staticfileserver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QTextStream>
#include <QWebSocketServer>

static QString projectRootPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 6; ++i) {
        if (dir.exists("config/server.conf")) {
            return dir.absolutePath();
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    return QCoreApplication::applicationDirPath();
}

static void readConfig(QString *host, quint16 *tcpPort, quint16 *wsPort, quint16 *httpPort)
{
    if (host) {
        *host = "127.0.0.1";
    }
    if (tcpPort) {
        *tcpPort = 8888;
    }
    if (wsPort) {
        *wsPort = 9999;
    }
    if (httpPort) {
        *httpPort = 8080;
    }

    QFile file(projectRootPath() + "/config/server.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.startsWith('#') || !line.contains('=')) {
            continue;
        }
        const QStringList parts = line.split('=');
        if (parts.size() != 2) {
            continue;
        }

        const QString key = parts[0].trimmed();
        const QString value = parts[1].trimmed();
        bool ok = false;
        const int parsed = value.toInt(&ok);

        if (key == "host" && host) {
            *host = value;
        } else if (key == "port" && tcpPort && ok) {
            *tcpPort = static_cast<quint16>(parsed);
        } else if (key == "ws_port" && wsPort && ok) {
            *wsPort = static_cast<quint16>(parsed);
        } else if (key == "http_port" && httpPort && ok) {
            *httpPort = static_cast<quint16>(parsed);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString host;
    quint16 tcpPort = 8888;
    quint16 wsPort = 9999;
    quint16 httpPort = 8080;
    readConfig(&host, &tcpPort, &wsPort, &httpPort);

    QWebSocketServer server("chat_web_gateway", QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::Any, wsPort)) {
        qCritical("Web gateway listen failed.");
        return -1;
    }

    StaticFileServer staticServer(projectRootPath() + "/web_client");
    if (!staticServer.listen(QHostAddress::Any, httpPort)) {
        qCritical("Static file server listen failed.");
        return -1;
    }

    QObject::connect(&server, &QWebSocketServer::newConnection, [&server, host, tcpPort]() {
        QWebSocket *socket = server.nextPendingConnection();
        if (!socket) {
            return;
        }
        new BridgeSession(socket, host, tcpPort, &server);
    });

    return app.exec();
}
