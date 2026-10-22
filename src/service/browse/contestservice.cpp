#include "contestservice.h"

#include "repository/cache/contestcacherepository.h"
#include "repository/submit/contestrepository.h"

namespace
{
bool sameProblems(const QList<ContestProblemInfo> &lhs,
                  const QList<ContestProblemInfo> &rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (int i = 0; i < lhs.size(); ++i) {
        const ContestProblemInfo &left = lhs.at(i);
        const ContestProblemInfo &right = rhs.at(i);
        if (left.problemId != right.problemId
            || left.title != right.title
            || left.problemUrl != right.problemUrl
            || left.accept_prople != right.accept_prople
            || left.submission_people != right.submission_people
            || left.solved != right.solved) {
            return false;
        }
    }

    return true;
}
}

ContestService::ContestService(ContestRepository *repository,
                               ContestCacheRepository *cacheRepository,
                               QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_cacheRepository(cacheRepository)
{
    connect(
        m_repository,
        &ContestRepository::problemsFetched,
        this,
        [this](const ContestPageInfo &contestPageInfo) {
            const bool changed = !m_hasLastContestInfo
                || !sameContestPage(m_lastContestInfo, contestPageInfo);
            if (m_cacheRepository != nullptr && changed) {
                m_cacheRepository->saveContest(contestPageInfo);
            }
            m_lastContestInfo = contestPageInfo;
            m_hasLastContestInfo = true;
            m_loading = false;
            emit loadingChanged(false);
            if (changed) {
                emit contestLoaded(contestPageInfo);
            }
        });

    connect(
        m_repository,
        &ContestRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            m_loading = false;
            emit loadingChanged(false);
            emit failed(message);
        });
}

void ContestService::openContest(const QUrl &contestPageUrl)
{
    m_currentContestUrl = contestPageUrl;
    if (m_cacheRepository != nullptr) {
        ContestPageInfo cachedContestInfo;
        if (m_cacheRepository->loadContest(
                contestPageUrl.toString(), &cachedContestInfo)) {
            m_lastContestInfo = cachedContestInfo;
            m_hasLastContestInfo = true;
            m_loading = false;
            emit loadingChanged(false);
            emit contestLoaded(cachedContestInfo);
        }
    }

    m_loading = true;
    emit loadingChanged(true);
    m_repository->fetchProblems(contestPageUrl);
}

QUrl ContestService::currentContestUrl() const
{
    return m_currentContestUrl;
}

bool ContestService::sameContestPage(const ContestPageInfo &lhs,
                                     const ContestPageInfo &rhs) const
{
    return lhs.contestPageUrl == rhs.contestPageUrl
        && sameProblems(lhs.problems, rhs.problems);
}
