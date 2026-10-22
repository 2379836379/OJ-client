#include "classrepository.h"

#include "network/openjudgeclient.h"

#include <QDebug>

ClassRepository::ClassRepository(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::classPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedClassPageUrl.isEmpty()
                && result.finalUrl != m_expectedClassPageUrl) {
                return;
            }

            m_expectedClassPageUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("classPage", result.errorString);
                return;
            }

            m_lastClassPageInfo =
                ClassParser::parseClassPage(result.body, result.finalUrl);
            if (m_lastClassPageInfo.groupEntryUrl == "not found") {
                emit requestFailed("classPage", "group entry url not found");
                return;
            }

            qDebug() << "group url =" << m_lastClassPageInfo.groupEntryUrl
                     << "course name =" << m_lastClassPageInfo.courseName;

            m_expectedGroupPageUrl = QUrl(m_lastClassPageInfo.groupEntryUrl);
            m_client->fetchGroupPage(m_expectedGroupPageUrl);
        });

    connect(
        m_client,
        &OpenJudgeClient::groupPageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_expectedGroupPageUrl.isEmpty()
                && result.finalUrl != m_expectedGroupPageUrl) {
                return;
            }

            m_expectedGroupPageUrl = QUrl();
            if (!result.ok) {
                emit requestFailed("groupPage", result.errorString);
                return;
            }

            emit contestSetsFetched(
                m_lastClassPageInfo,
                GroupParser::parseGroupPage(result.body, result.finalUrl));
        });
}

void ClassRepository::fetchContestSets(const QUrl &classPageUrl)
{
    m_expectedClassPageUrl = classPageUrl;
    m_client->fetchClassPage(classPageUrl);
}
