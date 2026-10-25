#include "GroupHomepageCache.h"

#include "PageCacheStore.h"

GroupHomepageCache::GroupHomepageCache()
    : m_store(new PageCacheStore("group_homepage_cache.db", "group_homepage_cache"))
{
}

GroupHomepageCache::~GroupHomepageCache() { delete m_store; }

bool GroupHomepageCache::load(const QString &homepageUrl, QString *groupUrl)
{
    QByteArray payload;
    if (!m_store->load(homepageUrl, &payload)) return false;
    const QString value = QString::fromUtf8(payload);
    if (value.isEmpty()) return false;
    if (groupUrl != nullptr) *groupUrl = value;
    return true;
}

bool GroupHomepageCache::save(const QString &homepageUrl, const QString &groupUrl)
{
    return m_store->save(homepageUrl, groupUrl.toUtf8());
}

qint64 GroupHomepageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool GroupHomepageCache::clearCache() { return m_store->clear(); }
QString GroupHomepageCache::lastError() const { return m_store->lastError(); }
