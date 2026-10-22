#include "classservice.h"

#include "repository/cache/classcacherepository.h"
#include "repository/submit/classrepository.h"

namespace
{
bool sameContestSets(const QList<ContestSetInfo> &lhs, const QList<ContestSetInfo> &rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (int i = 0; i < lhs.size(); ++i) {
        const ContestSetInfo &left = lhs.at(i);
        const ContestSetInfo &right = rhs.at(i);
        if (left.url != right.url || left.title != right.title
            || left.itemClass != right.itemClass
            || left.extraText != right.extraText) {
            return false;
        }
    }

    return true;
}

bool sameClassAndGroup(const ClassPageInfo &lhsClass,
                       const GroupPageInfo &lhsGroup,
                       const ClassPageInfo &rhsClass,
                       const GroupPageInfo &rhsGroup)
{
    return lhsClass.classPageUrl == rhsClass.classPageUrl
        && lhsClass.groupEntryUrl == rhsClass.groupEntryUrl
        && lhsClass.courseName == rhsClass.courseName
        && lhsGroup.groupPageUrl == rhsGroup.groupPageUrl
        && sameContestSets(lhsGroup.contestSets, rhsGroup.contestSets);
}
}

ClassService::ClassService(ClassRepository *repository,
                           ClassCacheRepository *cacheRepository,
                           QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_cacheRepository(cacheRepository)
{
    connect(
        m_repository,
        &ClassRepository::contestSetsFetched,
        this,
        [this](const ClassPageInfo &classPageInfo,
               const GroupPageInfo &groupPageInfo) {
            const bool changed = !m_hasLastLoadedData
                || !sameClassAndGroup(
                    m_lastClassInfo,
                    m_lastGroupInfo,
                    classPageInfo,
                    groupPageInfo);
            if (m_cacheRepository != nullptr && changed) {
                m_cacheRepository->saveClass(classPageInfo, groupPageInfo);
            }
            m_lastClassInfo = classPageInfo;
            m_lastGroupInfo = groupPageInfo;
            m_hasLastLoadedData = true;
            m_showingCachedData = false;
            m_loading = false;
            emit loadingChanged(false);
            if (changed) {
                emit classLoaded(classPageInfo, groupPageInfo);
            }
        });


    connect(
        m_repository,
        &ClassRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            m_loading = false;
            emit loadingChanged(false);
            if (!m_showingCachedData) {
                emit failed(message);
            }
        });
}

void ClassService::openClass(const QUrl &classPageUrl)
{
    m_currentClassUrl = classPageUrl;
    bool cacheHit = false;
    if (m_cacheRepository != nullptr) {
        ClassPageInfo cachedClassInfo;
        GroupPageInfo cachedGroupInfo;
        if (m_cacheRepository->loadClass(
                classPageUrl.toString(), &cachedClassInfo, &cachedGroupInfo)) {
            m_lastClassInfo = cachedClassInfo;
            m_lastGroupInfo = cachedGroupInfo;
            m_hasLastLoadedData = true;
            m_showingCachedData = true;
            emit classLoaded(cachedClassInfo, cachedGroupInfo);
            cacheHit = true;
        }
    }

    if (!cacheHit) {
        m_hasLastLoadedData = false;
        m_showingCachedData = false;
    }

    m_loading = true;
    emit loadingChanged(true);
    m_repository->fetchContestSets(classPageUrl);
}

QUrl ClassService::currentClassUrl() const
{
    return m_currentClassUrl;
}
