#include "cacheservice.h"

#include "repository/cache/classcacherepository.h"
#include "repository/cache/contestcacherepository.h"
#include "repository/cache/homecacherepository.h"
#include "repository/cache/problemcacherepository.h"

#include <QStringList>

CacheService::CacheService(HomeCacheRepository *homeCacheRepository,
                           ClassCacheRepository *classCacheRepository,
                           ContestCacheRepository *contestCacheRepository,
                           ProblemCacheRepository *problemCacheRepository,
                           QObject *parent)
    : QObject(parent)
    , m_homeCacheRepository(homeCacheRepository)
    , m_classCacheRepository(classCacheRepository)
    , m_contestCacheRepository(contestCacheRepository)
    , m_problemCacheRepository(problemCacheRepository)
{
}

qint64 CacheService::totalCacheSizeBytes() const
{
    qint64 total = 0;
    if (m_homeCacheRepository != nullptr) {
        total += m_homeCacheRepository->cacheSizeBytes();
    }
    if (m_classCacheRepository != nullptr) {
        total += m_classCacheRepository->cacheSizeBytes();
    }
    if (m_contestCacheRepository != nullptr) {
        total += m_contestCacheRepository->cacheSizeBytes();
    }
    if (m_problemCacheRepository != nullptr) {
        total += m_problemCacheRepository->cacheSizeBytes();
    }
    return total;
}

QString CacheService::formattedTotalCacheSize() const
{
    return formatBytes(totalCacheSizeBytes());
}

bool CacheService::clearAllCaches()
{
    QStringList errors;

    if (m_homeCacheRepository != nullptr && !m_homeCacheRepository->clearCache()) {
        errors << m_homeCacheRepository->lastError();
    }
    if (m_classCacheRepository != nullptr && !m_classCacheRepository->clearCache()) {
        errors << m_classCacheRepository->lastError();
    }
    if (m_contestCacheRepository != nullptr && !m_contestCacheRepository->clearCache()) {
        errors << m_contestCacheRepository->lastError();
    }
    if (m_problemCacheRepository != nullptr && !m_problemCacheRepository->clearCache()) {
        errors << m_problemCacheRepository->lastError();
    }

    if (!errors.isEmpty()) {
        m_lastError = errors.join("; ");
        emit failed(m_lastError);
        return false;
    }

    m_lastError.clear();
    emit cacheCleared();
    return true;
}

QString CacheService::lastError() const
{
    return m_lastError;
}

QString CacheService::formatBytes(qint64 bytes) const
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
