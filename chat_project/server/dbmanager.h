#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QMutex>
#include <QObject>
#include <QString>

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(const QString &dbPath, QObject *parent = nullptr);

    bool initDatabase(QString *errorMessage = nullptr);
    bool registerUser(const QString &username, const QString &password, QString *errorMessage = nullptr);
    bool validateUser(const QString &username, const QString &password, QString *errorMessage = nullptr);
    bool saveMessage(const QString &sender, const QString &receiver, const QString &content, const QString &sendTime, QString *errorMessage = nullptr);

private:
    QString connectionNameForCurrentThread() const;
    bool ensureConnection(QString *errorMessage = nullptr);

    QString m_dbPath;
    mutable QMutex m_mutex;
};

#endif
