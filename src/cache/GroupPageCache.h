#pragma once

#include "parser/GroupPage.h"

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class GroupPageCache
{
public:
    GroupPageCache();
    ~GroupPageCache();
    GroupPageCache(const GroupPageCache &) = delete;
    GroupPageCache &operator=(const GroupPageCache &) = delete;
    bool load(const QString &groupPageUrl, GroupPageInfo *pageInfo);
    bool save(const QString &groupPageUrl, const GroupPageInfo &pageInfo);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
