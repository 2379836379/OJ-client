#pragma once

#include "parser/UserHomepage.h"

#include <QString>
#include <QtGlobal>

class PageCacheStore;

class UserHomepageCache
{
public:
    UserHomepageCache();
    ~UserHomepageCache();
    UserHomepageCache(const UserHomepageCache &) = delete;
    UserHomepageCache &operator=(const UserHomepageCache &) = delete;
    bool load(const QString &userHomepageUrl, QList<JoinedClassInfo> *classes);
    bool save(const QString &userHomepageUrl, const QList<JoinedClassInfo> &classes);
    qint64 cacheSizeBytes() const;
    bool clearCache();
    QString lastError() const;

private:
    PageCacheStore *m_store = nullptr;
};
