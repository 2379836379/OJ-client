#include "problemrepository.h"

#include "network/openjudgeclient.h"

ProblemRepository::ProblemRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::problemPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedProblemUrl.isEmpty()
                && result.finalUrl != m_expectedProblemUrl) {
                return;
            }

            m_expectedProblemUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("problemPage", result.errorString);
                return;
            }

            emit problemFetched(
                ProblemParser::parseProblemPage(result.body, result.finalUrl));
        });
}

void ProblemRepository::fetchProblemDetail(const QUrl &problemUrl)
{
    m_expectedProblemUrl = problemUrl;
    m_client->fetchProblemPage(problemUrl);
}
