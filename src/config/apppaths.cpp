#include "config/apppaths.h"

#include <QDir>
#include <QStandardPaths>

namespace AppPaths
{
QString baseDir()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    const QString path = dir.filePath("OJ-client");
    QDir().mkpath(path);
    return path;
}

QString dataDir()
{
    const QString path = QDir(baseDir()).filePath("data");
    QDir().mkpath(path);
    return path;
}

QString logDir()
{
    const QString path = QDir(baseDir()).filePath("log");
    QDir().mkpath(path);
    return path;
}

QString cacheDir()
{
    const QString path = QDir(baseDir()).filePath("cache");
    QDir().mkpath(path);
    return path;
}

QString AIconfigFilePath()
{
    return QDir(baseDir()).filePath("AIconfig.toml");
}

QString appStateFilePath()
{
    return QDir(baseDir()).filePath("appconfig.toml");
}
}
