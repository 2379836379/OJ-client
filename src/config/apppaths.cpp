#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>

namespace
{
// Stable folder independent of QCoreApplication::applicationName (which the app
// deliberately blanks for window-title reasons).
QString appFolderName()
{
    return QStringLiteral("oj-client");
}

QDir legacyRootDir()
{
    QDir dir(QCoreApplication::applicationDirPath());
    if (dir.dirName().compare("build", Qt::CaseInsensitive) == 0) {
        dir.cdUp();
    } else if ((dir.dirName().compare("debug", Qt::CaseInsensitive) == 0
                || dir.dirName().compare("release", Qt::CaseInsensitive) == 0)
               && dir.cdUp()
               && dir.dirName().compare("build", Qt::CaseInsensitive) == 0) {
        dir.cdUp();
    }
    return dir;
}

QString firstExistingLegacyToml(const QString &fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    const QStringList candidates = {
        QDir(cwd).filePath(fileName),
        QDir(appDir).filePath(fileName),
        QDir(appDir).filePath("../" + fileName),
        QDir(appDir).filePath("../oj-client/" + fileName),
        QDir(appDir).filePath("../../oj-client/" + fileName)};
    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return QDir::cleanPath(path);
        }
    }
    return QString();
}

void copyIfDestMissing(const QString &src, const QString &dst)
{
    if (src.isEmpty() || QFile::exists(dst) || !QFile::exists(src)) {
        return;
    }
    QFile::copy(src, dst);
}

// On a fresh install the ringtone is bundled next to the executable; generate a
// default appstate.toml pointing at it (previously done by the Windows installer).
void writeDefaultAppStateIfMissing(const QString &appStatePath)
{
    if (QFile::exists(appStatePath)) {
        return;
    }
    const QString ringPath = QDir(QCoreApplication::applicationDirPath())
                                 .filePath("vedio/哈基米起床 - MyRingtone.mp3");
    if (!QFile::exists(ringPath)) {
        return;
    }
    QFile file(appStatePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    const QString content = QStringLiteral("ring_path = \"%1\"\n").arg(ringPath);
    file.write(content.toUtf8());
}
}

namespace AppPaths
{
QString baseDir()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    const QString path = dir.filePath(appFolderName());
    QDir().mkpath(path);
    return path;
}

QString dataDir()
{
    const QString path = QDir(baseDir()).filePath("data");
    QDir().mkpath(path);
    return path;
}

QString cacheDir()
{
    const QString path = QDir(baseDir()).filePath("cache");
    QDir().mkpath(path);
    return path;
}

QString configFilePath()
{
    return QDir(baseDir()).filePath("config.toml");
}

QString appStateFilePath()
{
    return QDir(baseDir()).filePath("appstate.toml");
}

void migrateLegacyDataIfNeeded()
{
    const QDir legacyRoot = legacyRootDir();
    const QDir legacyData(legacyRoot.filePath("data"));
    const QString newData = dataDir();

    const QStringList keyDatabases = {
        "favorites.db", "problemmeta.db", "login_cache.db"};
    for (const QString &name : keyDatabases) {
        copyIfDestMissing(legacyData.filePath(name), QDir(newData).filePath(name));
    }

    copyIfDestMissing(firstExistingLegacyToml("config.toml"), configFilePath());
    copyIfDestMissing(firstExistingLegacyToml("appstate.toml"), appStateFilePath());

    writeDefaultAppStateIfMissing(appStateFilePath());
}
}
