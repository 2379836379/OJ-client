#include "homerepository.h"

#include "network/openjudgeclient.h"
#include "parser/UserHomePageParser.h"

HomeRepository::HomeRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::userHomePageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedUserHomeUrl.isEmpty()
                && result.finalUrl != m_expectedUserHomeUrl) {
                return;
            }

            m_expectedUserHomeUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("userHomePage", result.errorString);
                return;
            }

            emit joinedClassesFetched(
                ClassParser::extractJoinedClasses(result.body));
        });
}

void HomeRepository::fetchJoinedClasses(const QUrl &userHomeUrl)
{
    m_expectedUserHomeUrl = userHomeUrl;
    m_client->fetchUserHomePage(userHomeUrl);
}
