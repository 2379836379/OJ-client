#pragma once

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class GroupHomepageCache
{
public:
    GroupHomepageCache();
    ~GroupHomepageCache();
    GroupHomepageCache(const GroupHomepageCache &) = delete;
    GroupHomepageCache &operator=(const GroupHomepageCache &) = delete;
    bool load(const QString &homepageUrl, QString *groupUrl);
    bool save(const QString &homepageUrl, const QString &groupUrl);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
