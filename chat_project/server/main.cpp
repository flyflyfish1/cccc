#include "chatserver.h"
#include "dbmanager.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QTextStream>

static QString ensureDir(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.absolutePath();
}

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

static quint16 readPortFromConfig(const QString &configPath)
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 8888;
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
        if (parts[0].trimmed() == "port") {
            bool ok = false;
            const int port = parts[1].trimmed().toInt(&ok);
            if (ok && port > 0 && port < 65536) {
                return static_cast<quint16>(port);
            }
        }
    }
    return 8888;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const QString rootPath = projectRootPath();
    ensureDir(rootPath + "/logs");
    ensureDir(rootPath + "/config");
    ensureDir(rootPath + "/database");

    Logger::instance().init(rootPath + "/logs/server.log");
    Logger::instance().write("Server is starting.");

    DBManager dbManager(rootPath + "/database/chat.db");
    QString dbError;
    if (!dbManager.initDatabase(&dbError)) {
        Logger::instance().write(QString("Database init failed: %1").arg(dbError));
        return -1;
    }

    ChatServer server;
    server.setDbManager(&dbManager);

    const quint16 port = readPortFromConfig(rootPath + "/config/server.conf");
    if (!server.listen(QHostAddress::Any, port)) {
        Logger::instance().write(QString("Listen failed: %1").arg(server.errorString()));
        return -1;
    }

    Logger::instance().write(QString("Server started at port %1").arg(port));
    return a.exec();
}
