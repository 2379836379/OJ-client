#include "resultrepository.h"

#include "network/openjudgeclient.h"

ResultRepository::ResultRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::resultPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedResultPageUrl.isEmpty()
                && result.finalUrl != m_expectedResultPageUrl) {
                return;
            }

            m_expectedResultPageUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("resultPage", result.errorString);
                return;
            }

            emit resultPageFetched(
                ResultParser::parseResultPage(result.body, result.finalUrl));
        });
}

void ResultRepository::fetchResultPage(const QUrl &resultPageUrl)
{
    m_expectedResultPageUrl = resultPageUrl;
    m_client->fetchResultPage(resultPageUrl);
}
