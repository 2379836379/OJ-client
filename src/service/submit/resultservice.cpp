#include "resultservice.h"

#include "repository/submit/resultrepository.h"

namespace
{
bool isWaitingStatus(const QString &statusText)
{
    return statusText.compare("Waiting", Qt::CaseInsensitive) == 0;
}
}

ResultService::ResultService(ResultRepository *repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
        if (m_currentResultUrl.isEmpty()) {
            return;
        }

        m_loading = true;
        emit loadingChanged(true);
        m_repository->fetchResultPage(m_currentResultUrl);
    });

    connect(
        m_repository,
        &ResultRepository::resultPageFetched,
        this,
        [this](const ResultPageInfo &resultPageInfo) {
            emit resultLoaded(resultPageInfo);

            if (isWaitingStatus(resultPageInfo.statusText)) {
                scheduleRetry();
                return;
            }

            m_retryTimer.stop();
            m_loading = false;
            emit loadingChanged(false);
        });

    connect(
        m_repository,
        &ResultRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            m_retryTimer.stop();
            m_loading = false;
            emit loadingChanged(false);
            emit failed(message);
        });
}

void ResultService::openResult(const QUrl &resultPageUrl)
{
    m_retryTimer.stop();
    m_currentResultUrl = resultPageUrl;
    m_loading = true;
    emit loadingChanged(true);
    m_repository->fetchResultPage(resultPageUrl);
}

QUrl ResultService::currentResultUrl() const
{
    return m_currentResultUrl;
}

void ResultService::scheduleRetry()
{
    if (!m_retryTimer.isActive()) {
        m_retryTimer.start(2000);
    }
}
