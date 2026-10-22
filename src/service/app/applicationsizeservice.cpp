#include "service/app/applicationsizeservice.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace
{
QDir projectRootDir()
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
}

ApplicationSizeService::ApplicationSizeService(QObject *parent)
    : QObject(parent)
{
}

qint64 ApplicationSizeService::totalApplicationSizeBytes() const
{
    m_lastError.clear();
    // Install directory (binaries/resources) plus the per-user data directory
    // (databases, caches, toml) which now lives outside the install location.
    return directorySizeBytes(projectRootDir().absolutePath())
           + directorySizeBytes(AppPaths::baseDir());
}

QString ApplicationSizeService::formattedTotalApplicationSize() const
{
    return formatBytes(totalApplicationSizeBytes());
}

QString ApplicationSizeService::lastError() const
{
    return m_lastError;
}

qint64 ApplicationSizeService::directorySizeBytes(const QString &path) const
{
    QFileInfo rootInfo(path);
    if (!rootInfo.exists()) {
        m_lastError = QString("Path not found: %1").arg(path);
        return 0;
    }

    qint64 total = 0;
    QDirIterator iterator(path,
                          QDir::Files | QDir::Hidden | QDir::System,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        total += iterator.fileInfo().size();
    }
    return total;
}

QString ApplicationSizeService::formatBytes(qint64 bytes) const
{
    static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 4) {
        value /= 1024.0;
        ++unitIndex;
    }

    return unitIndex == 0
        ? QString("%1 %2").arg(bytes).arg(units[unitIndex])
        : QString("%1 %2").arg(QString::number(value, 'f', 2)).arg(units[unitIndex]);
}
