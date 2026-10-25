#pragma once

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class HomepageCache
{
public:
    HomepageCache();
    ~HomepageCache();
    HomepageCache(const HomepageCache &) = delete;
    HomepageCache &operator=(const HomepageCache &) = delete;
    bool load(QString *personalHomepageUrl);
    bool save(const QString &personalHomepageUrl);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
