#include "contestrepository.h"

#include "network/openjudgeclient.h"

ContestRepository::ContestRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::contestPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedContestPageUrl.isEmpty()
                && result.finalUrl != m_expectedContestPageUrl) {
                return;
            }

            m_expectedContestPageUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("contestPage", result.errorString);
                return;
            }

            emit problemsFetched(
                ContestParser::parseContestPage(result.body, result.finalUrl));
        });
}

void ContestRepository::fetchProblems(const QUrl &contestPageUrl)
{
    m_expectedContestPageUrl = contestPageUrl;
    m_client->fetchContestPage(contestPageUrl);
}
