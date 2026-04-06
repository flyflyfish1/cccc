#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QTextStream>

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::init(const QString &logFilePath)
{
    QMutexLocker locker(&m_mutex);
    m_logFilePath = logFilePath;
    QFileInfo info(logFilePath);
    QDir dir = info.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void Logger::write(const QString &message)
{
    QMutexLocker locker(&m_mutex);
    if (m_logFilePath.isEmpty()) {
        return;
    }

    QFile file(m_logFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "] "
        << message << "\n";
}
