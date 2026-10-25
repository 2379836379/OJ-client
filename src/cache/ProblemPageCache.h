#pragma once

#include "parser/ProblemPage.h"

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class ProblemPageCache
{
public:
    ProblemPageCache();
    ~ProblemPageCache();
    ProblemPageCache(const ProblemPageCache &) = delete;
    ProblemPageCache &operator=(const ProblemPageCache &) = delete;
    bool load(const QString &problemPageUrl, ProblemPageInfo *pageInfo);
    bool save(const QString &problemPageUrl, const ProblemPageInfo &pageInfo);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
