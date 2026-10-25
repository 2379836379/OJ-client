#include "UserHomepage.h"

#include "cache/UserHomepageCache.h"
#include "network/openjudgeclient.h"
#include "parser/UserHomepage.h"

namespace Browse
{
UserHomepage::UserHomepage(OpenJudgeClient *client, UserHomepageCache *cache,
                           QObject *parent)
    : QObject(parent), m_client(client), m_cache(cache)
{
    if (!m_client) return;
    connect(m_client, &OpenJudgeClient::userHomePageFinished, this,
            [this](const NetworkResult &result) {
                if (result.requestUrl != m_stateUrl) return;
                emit loadingChanged(false);
                if (!result.ok) {
                    if (!m_cacheHit) emit failed(result.errorString);
                    return;
                }
                m_cacheHit = false;
                const auto classes = UserHomepageParser::parseUserHomepage(result.body);
                if (m_cache) m_cache->save(m_stateUrl.toString(), classes);
                emit loaded(classes);
            });
}

void UserHomepage::open(const QUrl &url)
{
    m_stateUrl = url;
    QList<JoinedClassInfo> classes;
    m_cacheHit = m_cache && m_cache->load(url.toString(), &classes);
    if (m_cacheHit) emit loaded(classes);
    emit loadingChanged(true);
    if (m_client) m_client->fetchUserHomePage(url);
    else {
        emit loadingChanged(false);
        if (!m_cacheHit) emit failed(QStringLiteral("OpenJudge client is unavailable."));
    }
}
}
