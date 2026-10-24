#pragma once

#include <QObject>
#include <QString>
#include <QtGlobal>

class HomeCacheRepository;
class ClassCacheRepository;
class ContestCacheRepository;
class ProblemCacheRepository;

class CacheService : public QObject
{
    Q_OBJECT

public:
    explicit CacheService(HomeCacheRepository *homeCacheRepository,
                          ClassCacheRepository *classCacheRepository,
                          ContestCacheRepository *contestCacheRepository,
                          ProblemCacheRepository *problemCacheRepository,
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

    HomeCacheRepository *m_homeCacheRepository = nullptr;
    ClassCacheRepository *m_classCacheRepository = nullptr;
    ContestCacheRepository *m_contestCacheRepository = nullptr;
    ProblemCacheRepository *m_problemCacheRepository = nullptr;
    QString m_lastError;
};
