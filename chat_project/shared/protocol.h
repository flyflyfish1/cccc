#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace Protocol {

QString currentTimeString();
QJsonObject createRegisterRequest(const QString &username, const QString &password);
QJsonObject createLoginRequest(const QString &username, const QString &password);
QJsonObject createChatMessage(const QString &from, const QString &to, const QString &message);
QJsonObject createUserListMessage(const QStringList &users);
QJsonObject createSimpleResponse(const QString &type, const QString &status, const QString &message);
QByteArray packJson(const QJsonObject &obj);
QList<QJsonObject> unpackMessages(QByteArray &buffer);
QString jsonToDisplayString(const QJsonObject &obj);

}

#endif
