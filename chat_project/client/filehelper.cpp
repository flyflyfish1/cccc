#include "filehelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

QString FileHelper::projectRootPath()
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

QString FileHelper::ensureDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.absolutePath();
}

bool FileHelper::saveLocalHistory(const QString &selfUser, const QString &peerUser, const QString &line)
{
    const QString historyDir = ensureDirectory(projectRootPath() + "/history");
    const QString fileName = QString("%1_%2.txt").arg(selfUser, peerUser);
    QFile file(historyDir + "/" + fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out << line << "\n";
    return true;
}

bool FileHelper::readServerConfig(QString *host, quint16 *port)
{
    if (host) {
        *host = "127.0.0.1";
    }
    if (port) {
        *port = 8888;
    }

    QFile file(projectRootPath() + "/config/server.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
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
        if (key == "host" && host) {
            *host = value;
        } else if (key == "port" && port) {
            bool ok = false;
            const int v = value.toInt(&ok);
            if (ok && v > 0 && v < 65536) {
                *port = static_cast<quint16>(v);
            }
        }
    }
    return true;
}
