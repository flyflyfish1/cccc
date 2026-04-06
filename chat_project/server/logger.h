#ifndef LOGGER_H
#define LOGGER_H

#include <QMutex>
#include <QString>

class Logger
{
public:
    static Logger &instance();
    void init(const QString &logFilePath);
    void write(const QString &message);

private:
    Logger() = default;

    QString m_logFilePath;
    QMutex m_mutex;
};

#endif
