#include "Homepage.h"

#include "cache/HomepageCache.h"
#include "network/openjudgeclient.h"
#include "parser/Homepage.h"

namespace Browse
{
Homepage::Homepage(OpenJudgeClient *client, HomepageCache *cache, QObject *parent)
    : QObject(parent), m_client(client), m_cache(cache)
{
    if (!m_client) return;
    connect(m_client, &OpenJudgeClient::homePageFinished, this,
            [this](const NetworkResult &result) {
                if (!result.ok) {
                    if (!m_cacheHit) emit failed(result.errorString);
                    return;
                }
                const QString url = HomepageParser::parseHomepage(result.body);
                if (url.isEmpty()) {
                    if (!m_cacheHit) emit failed(QStringLiteral("Personal homepage URL not found."));
                    return;
                }
                m_cacheHit = false;
                if (m_cache) m_cache->save(url);
                emit loaded(url);
            });
}

void Homepage::open()
{
    QString cachedUrl;
    m_cacheHit = m_cache && m_cache->load(&cachedUrl);
    if (m_cacheHit) emit loaded(cachedUrl);
    if (m_client) m_client->fetchHomePage();
    else if (!m_cacheHit) emit failed(QStringLiteral("OpenJudge client is unavailable."));
}
}
