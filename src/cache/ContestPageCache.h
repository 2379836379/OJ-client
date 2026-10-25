#pragma once

#include "parser/ContestPage.h"

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class ContestPageCache
{
public:
    ContestPageCache();
    ~ContestPageCache();
    ContestPageCache(const ContestPageCache &) = delete;
    ContestPageCache &operator=(const ContestPageCache &) = delete;
    bool load(const QString &contestPageUrl, ContestPageInfo *pageInfo);
    bool save(const QString &contestPageUrl, const ContestPageInfo &pageInfo);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
