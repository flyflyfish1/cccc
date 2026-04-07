#include "dbmanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>
#include <QMutexLocker>

DBManager::DBManager(const QString &dbPath, QObject *parent)
    : QObject(parent),
      m_dbPath(dbPath)
{
}

QString DBManager::connectionNameForCurrentThread() const
{
    return QString("server_db_%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
}

bool DBManager::ensureConnection(QString *errorMessage)
{
    const QString connectionName = connectionNameForCurrentThread();
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen() || db.open()) {
            return true;
        }
        if (errorMessage) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        if (errorMessage) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }
    return true;
}

bool DBManager::initDatabase(QString *errorMessage)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureConnection(errorMessage)) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionNameForCurrentThread());
    QSqlQuery query(db);

    const QString createUsersSql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL"
        ")";
    const QString createMessagesSql =
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender TEXT NOT NULL,"
        "receiver TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "send_time TEXT NOT NULL"
        ")";

    if (!query.exec(createUsersSql)) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    if (!query.exec(createMessagesSql)) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool DBManager::registerUser(const QString &username, const QString &password, QString *errorMessage)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureConnection(errorMessage)) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionNameForCurrentThread());
    QSqlQuery query(db);
    query.prepare("INSERT INTO users(username, password) VALUES(:username, :password)");
    query.bindValue(":username", QVariant(username));
    query.bindValue(":password", QVariant(password));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool DBManager::validateUser(const QString &username, const QString &password, QString *errorMessage)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureConnection(errorMessage)) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionNameForCurrentThread());
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", QVariant(username));
    query.bindValue(":password", QVariant(password));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.next();
}

bool DBManager::saveMessage(const QString &sender, const QString &receiver, const QString &content, const QString &sendTime, QString *errorMessage)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureConnection(errorMessage)) {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionNameForCurrentThread());
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO messages(sender, receiver, content, send_time) "
        "VALUES(:sender, :receiver, :content, :send_time)");
    query.bindValue(":sender", QVariant(sender));
    query.bindValue(":receiver", QVariant(receiver));
    query.bindValue(":content", QVariant(content));
    query.bindValue(":send_time", QVariant(sendTime));
    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

QJsonArray DBManager::loadConversation(const QString &userA, const QString &userB, QString *errorMessage)
{
    QMutexLocker locker(&m_mutex);
    QJsonArray result;

    if (!ensureConnection(errorMessage)) {
        return result;
    }

    QSqlDatabase db = QSqlDatabase::database(connectionNameForCurrentThread());
    QSqlQuery query(db);
    query.prepare(
        "SELECT sender, receiver, content, send_time "
        "FROM messages "
        "WHERE (sender = :userA AND receiver = :userB) "
        "   OR (sender = :userB AND receiver = :userA) "
        "ORDER BY id ASC");
    query.bindValue(":userA", QVariant(userA));
    query.bindValue(":userB", QVariant(userB));

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return result;
    }

    while (query.next()) {
        QJsonObject item;
        item["sender"] = query.value(0).toString();
        item["receiver"] = query.value(1).toString();
        item["content"] = query.value(2).toString();
        item["time"] = query.value(3).toString();
        result.append(item);
    }

    return result;
}
