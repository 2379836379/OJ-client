#include "HomepageCache.h"

#include "PageCacheStore.h"

HomepageCache::HomepageCache()
    : m_store(new PageCacheStore("homepage_cache.db", "homepage_cache"))
{
}

HomepageCache::~HomepageCache() { delete m_store; }

bool HomepageCache::load(QString *personalHomepageUrl)
{
    QByteArray payload;
    if (!m_store->load("homepage", &payload)) return false;
    const QString value = QString::fromUtf8(payload);
    if (value.isEmpty()) return false;
    if (personalHomepageUrl != nullptr) *personalHomepageUrl = value;
    return true;
}

bool HomepageCache::save(const QString &personalHomepageUrl)
{
    return m_store->save("homepage", personalHomepageUrl.toUtf8());
}

qint64 HomepageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool HomepageCache::clearCache() { return m_store->clear(); }
QString HomepageCache::lastError() const { return m_store->lastError(); }
