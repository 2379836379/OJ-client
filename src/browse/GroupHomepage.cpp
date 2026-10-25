#include "GroupHomepage.h"

#include "cache/GroupHomepageCache.h"
#include "network/openjudgeclient.h"
#include "parser/GroupHomepage.h"

namespace Browse
{
GroupHomepage::GroupHomepage(OpenJudgeClient *client, GroupHomepageCache *cache,
                             QObject *parent)
    : QObject(parent), m_client(client), m_cache(cache)
{
    if (!m_client) return;
    connect(m_client, &OpenJudgeClient::GroupHomePageFinished, this,
            [this](const NetworkResult &result) {
                if (result.requestUrl != m_url) return;
                emit loadingChanged(false);
                if (!result.ok) {
                    if (!m_cacheHit) emit failed(result.errorString);
                    return;
                }
                const QString path =
                    GroupHomepageParser::parseGroupHomepage(result.body);
                if (path.isEmpty()) {
                    if (!m_cacheHit) emit failed(QStringLiteral("Group URL not found."));
                    return;
                }
                const QUrl groupUrl = result.finalUrl.resolved(QUrl(path));
                if (!groupUrl.isValid() || groupUrl.scheme().isEmpty()) {
                    if (!m_cacheHit) emit failed(QStringLiteral("Group URL is invalid."));
                    return;
                }
                m_cacheHit = false;
                if (m_cache) m_cache->save(m_url.toString(), groupUrl.toString());
                emit loaded(groupUrl.toString());
            });
}

void GroupHomepage::open(const QUrl &url)
{
    m_url = url;
    QString groupUrl;
    m_cacheHit = m_cache && m_cache->load(url.toString(), &groupUrl);
    if (m_cacheHit) emit loaded(groupUrl);
    emit loadingChanged(true);
    if (m_client) m_client->fetchGroupHomePage(url);
    else {
        emit loadingChanged(false);
        if (!m_cacheHit) emit failed(QStringLiteral("OpenJudge client is unavailable."));
    }
}
}
