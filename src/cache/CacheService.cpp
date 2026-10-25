#include "CacheService.h"

#include "ContestPageCache.h"
#include "GroupHomepageCache.h"
#include "GroupPageCache.h"
#include "HomepageCache.h"
#include "ProblemPageCache.h"
#include "UserHomepageCache.h"

#include <QStringList>

CacheService::CacheService(HomepageCache *homepageCache,
                           UserHomepageCache *userHomepageCache,
                           GroupHomepageCache *groupHomepageCache,
                           GroupPageCache *groupPageCache,
                           ContestPageCache *contestPageCache,
                           ProblemPageCache *problemPageCache,
                           QObject *parent)
    : QObject(parent)
    , m_homepageCache(homepageCache)
    , m_userHomepageCache(userHomepageCache)
    , m_groupHomepageCache(groupHomepageCache)
    , m_groupPageCache(groupPageCache)
    , m_contestPageCache(contestPageCache)
    , m_problemPageCache(problemPageCache)
{
}

qint64 CacheService::totalCacheSizeBytes() const
{
    qint64 total = 0;
    if (m_homepageCache) total += m_homepageCache->cacheSizeBytes();
    if (m_userHomepageCache) total += m_userHomepageCache->cacheSizeBytes();
    if (m_groupHomepageCache) total += m_groupHomepageCache->cacheSizeBytes();
    if (m_groupPageCache) total += m_groupPageCache->cacheSizeBytes();
    if (m_contestPageCache) total += m_contestPageCache->cacheSizeBytes();
    if (m_problemPageCache) total += m_problemPageCache->cacheSizeBytes();
    return total;
}

QString CacheService::formattedTotalCacheSize() const
{
    return formatBytes(totalCacheSizeBytes());
}

bool CacheService::clearAllCaches()
{
    QStringList errors;
    auto clear = [&errors](auto *cache) {
        if (cache != nullptr && !cache->clearCache()) errors << cache->lastError();
    };
    clear(m_homepageCache);
    clear(m_userHomepageCache);
    clear(m_groupHomepageCache);
    clear(m_groupPageCache);
    clear(m_contestPageCache);
    clear(m_problemPageCache);

    if (!errors.isEmpty()) {
        m_lastError = errors.join("; ");
        emit failed(m_lastError);
        return false;
    }
    m_lastError.clear();
    emit cacheCleared();
    return true;
}

QString CacheService::lastError() const { return m_lastError; }

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
