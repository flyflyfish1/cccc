#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QString>

class FileHelper
{
public:
    static QString projectRootPath();
    static QString ensureDirectory(const QString &dirPath);
    static bool saveLocalHistory(const QString &selfUser, const QString &peerUser, const QString &line);
    static bool readServerConfig(QString *host, quint16 *port);
};

#endif
