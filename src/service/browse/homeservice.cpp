#include "homeservice.h"

#include "repository/cache/homecacherepository.h"
#include "repository/submit/homerepository.h"

namespace
{
bool sameClasses(const QList<JoinedClassInfo> &lhs, const QList<JoinedClassInfo> &rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (int i = 0; i < lhs.size(); ++i) {
        if (lhs.at(i).name != rhs.at(i).name || lhs.at(i).url != rhs.at(i).url) {
            return false;
        }
    }

    return true;
}
}

HomeService::HomeService(HomeRepository *repository,
                         HomeCacheRepository *cacheRepository,
                         QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_cacheRepository(cacheRepository)
{
    connect(
        m_repository,
        &HomeRepository::joinedClassesFetched,
        this,
        [this](const QList<JoinedClassInfo> &classes) {
            const bool changed = !m_hasLastLoadedClasses
                || !sameClasses(m_lastLoadedClasses, classes);
            if (m_cacheRepository != nullptr && changed) {
                m_cacheRepository->saveHome(m_currentHomeUrl.toString(), classes);
            }
            m_lastLoadedClasses = classes;
            m_hasLastLoadedClasses = true;
            m_showingCachedData = false;
            m_loading = false;
            emit loadingChanged(false);
            if (changed) {
                emit homeLoaded(classes);
            }
        });

    connect(
        m_repository,
        &HomeRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            m_loading = false;
            emit loadingChanged(false);
            if (!m_showingCachedData) {
                emit failed(message);
            }
        });
}

void HomeService::openHome(const QUrl &userHomeUrl)
{
    m_currentHomeUrl = userHomeUrl;
    bool cacheHit = false;
    if (m_cacheRepository != nullptr) {
        QList<JoinedClassInfo> cachedClasses;
        if (m_cacheRepository->loadHome(userHomeUrl.toString(), &cachedClasses)) {
            m_lastLoadedClasses = cachedClasses;
            m_hasLastLoadedClasses = true;
            m_showingCachedData = true;
            emit homeLoaded(cachedClasses);
            cacheHit = true;
        }
    }

    if (!cacheHit) {
        m_hasLastLoadedClasses = false;
        m_showingCachedData = false;
    }

    m_loading = true;
    emit loadingChanged(true);
    m_repository->fetchJoinedClasses(userHomeUrl);
}

QUrl HomeService::currentHomeUrl() const
{
    return m_currentHomeUrl;
}
