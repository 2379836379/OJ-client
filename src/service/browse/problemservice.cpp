#include "problemservice.h"

#include "repository/cache/problemcacherepository.h"
#include "repository/submit/problemrepository.h"

ProblemService::ProblemService(ProblemRepository *repository,
                               ProblemCacheRepository *cacheRepository,
                               QObject *parent)
    : QObject(parent)
    , m_repository(repository)
    , m_cacheRepository(cacheRepository)
{
    connect(
        m_repository,
        &ProblemRepository::problemFetched,
        this,
        [this](const ProblemPageInfo &problemPageInfo) {
            const bool changed = !m_hasLastProblemInfo
                || !sameProblemPage(m_lastProblemInfo, problemPageInfo);
            if (m_cacheRepository != nullptr && changed) {
                m_cacheRepository->saveProblem(problemPageInfo);
            }
            m_lastProblemInfo = problemPageInfo;
            m_hasLastProblemInfo = true;
            m_showingCachedData = false;
            m_loading = false;
            emit loadingChanged(false);
            if (changed) {
                emit problemLoaded(problemPageInfo);
            }
        });

    connect(
        m_repository,
        &ProblemRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            m_loading = false;
            emit loadingChanged(false);
            if (!m_showingCachedData) {
                emit failed(message);
            }
        });
}

void ProblemService::openProblem(const QUrl &problemUrl)
{
    m_currentProblemUrl = problemUrl;
    bool cacheHit = false;
    if (m_cacheRepository != nullptr) {
        ProblemPageInfo cachedProblemInfo;
        if (m_cacheRepository->loadProblem(problemUrl.toString(), &cachedProblemInfo)) {
            m_lastProblemInfo = cachedProblemInfo;
            m_hasLastProblemInfo = true;
            m_showingCachedData = true;
            emit problemLoaded(cachedProblemInfo);
            cacheHit = true;
        }
    }

    if (!cacheHit) {
        m_hasLastProblemInfo = false;
        m_showingCachedData = false;
    }

    m_loading = true;
    emit loadingChanged(true);
    m_repository->fetchProblemDetail(problemUrl);
}

QUrl ProblemService::currentProblemUrl() const
{
    return m_currentProblemUrl;
}

bool ProblemService::sameProblemPage(const ProblemPageInfo &lhs,
                                     const ProblemPageInfo &rhs) const
{
    return lhs.problemUrl == rhs.problemUrl
        && lhs.title == rhs.title
        && lhs.submitUrl == rhs.submitUrl
        && lhs.timeLimit == rhs.timeLimit
        && lhs.memoryLimit == rhs.memoryLimit
        && lhs.description == rhs.description
        && lhs.starterCode == rhs.starterCode
        && lhs.inputSpec == rhs.inputSpec
        && lhs.outputSpec == rhs.outputSpec
        && lhs.sampleInput == rhs.sampleInput
        && lhs.sampleOutput == rhs.sampleOutput
        && lhs.hint == rhs.hint
        && lhs.tried_people == rhs.tried_people
        && lhs.passed_people == rhs.passed_people;
}
