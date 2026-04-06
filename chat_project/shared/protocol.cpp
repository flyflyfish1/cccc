#include "protocol.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

namespace Protocol {

QString currentTimeString()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

QJsonObject createRegisterRequest(const QString &username, const QString &password)
{
    QJsonObject obj;
    obj["type"] = "register";
    obj["username"] = username;
    obj["password"] = password;
    obj["time"] = currentTimeString();
    return obj;
}

QJsonObject createLoginRequest(const QString &username, const QString &password)
{
    QJsonObject obj;
    obj["type"] = "login";
    obj["username"] = username;
    obj["password"] = password;
    obj["time"] = currentTimeString();
    return obj;
}

QJsonObject createChatMessage(const QString &from, const QString &to, const QString &message)
{
    QJsonObject obj;
    obj["type"] = "chat";
    obj["from"] = from;
    obj["to"] = to;
    obj["message"] = message;
    obj["time"] = currentTimeString();
    return obj;
}

QJsonObject createUserListMessage(const QStringList &users)
{
    QJsonObject obj;
    obj["type"] = "userlist";
    obj["time"] = currentTimeString();

    QJsonArray array;
    for (const QString &user : users) {
        array.append(user);
    }
    obj["users"] = array;
    return obj;
}

QJsonObject createSimpleResponse(const QString &type, const QString &status, const QString &message)
{
    QJsonObject obj;
    obj["type"] = type;
    obj["status"] = status;
    obj["message"] = message;
    obj["time"] = currentTimeString();
    return obj;
}

QByteArray packJson(const QJsonObject &obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact) + '\n';
}

QList<QJsonObject> unpackMessages(QByteArray &buffer)
{
    QList<QJsonObject> result;
    while (true) {
        int index = buffer.indexOf('\n');
        if (index < 0) {
            break;
        }

        const QByteArray line = buffer.left(index).trimmed();
        buffer.remove(0, index + 1);
        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            result.append(doc.object());
        }
    }
    return result;
}

QString jsonToDisplayString(const QJsonObject &obj)
{
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

}
