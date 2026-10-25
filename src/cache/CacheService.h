#pragma once

#include <QObject>
#include <QString>
#include <QtGlobal>

class HomepageCache;
class UserHomepageCache;
class GroupHomepageCache;
class GroupPageCache;
class ContestPageCache;
class ProblemPageCache;

class CacheService : public QObject
{
    Q_OBJECT

public:
    explicit CacheService(HomepageCache *homepageCache,
                          UserHomepageCache *userHomepageCache,
                          GroupHomepageCache *groupHomepageCache,
                          GroupPageCache *groupPageCache,
                          ContestPageCache *contestPageCache,
                          ProblemPageCache *problemPageCache,
                          QObject *parent = nullptr);

    qint64 totalCacheSizeBytes() const;
    QString formattedTotalCacheSize() const;
    bool clearAllCaches();
    QString lastError() const;

signals:
    void cacheCleared();
    void failed(const QString &message);

private:
    QString formatBytes(qint64 bytes) const;

    HomepageCache *m_homepageCache = nullptr;
    UserHomepageCache *m_userHomepageCache = nullptr;
    GroupHomepageCache *m_groupHomepageCache = nullptr;
    GroupPageCache *m_groupPageCache = nullptr;
    ContestPageCache *m_contestPageCache = nullptr;
    ProblemPageCache *m_problemPageCache = nullptr;
    QString m_lastError;
};
