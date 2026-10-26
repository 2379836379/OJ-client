#include "resultservice.h"

#include "network/openjudgeclient.h"

namespace
{
bool isWaitingStatus(const QString &statusText)
{
    return statusText.compare("Waiting", Qt::CaseInsensitive) == 0;
}
}

ResultService::ResultService(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
        if (m_currentResultUrl.isEmpty()) {
            return;
        }

        m_expectedResultUrl = m_currentResultUrl;
        emit loadingChanged(true);
        m_client->fetchResultPage(m_currentResultUrl);
    });

    connect(
        m_client,
        &OpenJudgeClient::resultPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedResultUrl.isEmpty()
                && result.requestUrl != m_expectedResultUrl) {
                return;
            }
            m_expectedResultUrl = QUrl();
            if (!result.ok) {
                m_retryTimer.stop();
                emit loadingChanged(false);
                emit failed(result.errorString);
                return;
            }

            const ResultPageInfo resultPageInfo =
                ResultPageParser::parseResultPage(result.body);
            emit resultLoaded(resultPageInfo);

            if (isWaitingStatus(resultPageInfo.statusText)) {
                scheduleRetry();
                return;
            }

            m_retryTimer.stop();
            emit loadingChanged(false);
        });
}

void ResultService::openResult(const QUrl &resultPageUrl)
{
    m_retryTimer.stop();
    m_currentResultUrl = resultPageUrl;
    m_expectedResultUrl = resultPageUrl;
    emit loadingChanged(true);
    m_client->fetchResultPage(resultPageUrl);
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
