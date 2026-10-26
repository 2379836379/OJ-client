#include "ProblemPage.h"

#include "cache/ProblemPageCache.h"
#include "network/openjudgeclient.h"
#include "parser/ProblemPage.h"

namespace Browse
{
ProblemPage::ProblemPage(OpenJudgeClient *client, ProblemPageCache *cache, QObject *parent)
    : QObject(parent), m_client(client), m_cache(cache)
{
    if (!m_client) return;
    connect(m_client, &OpenJudgeClient::problemPageFinished, this,
            [this](const NetworkResult &result) {
                if (result.requestUrl != m_url) return;
                emit loadingChanged(false);
                if (!result.ok) {
                    if (!m_cacheHit) emit failed(result.errorString);
                    return;
                }
                m_cacheHit = false;
                const ProblemPageInfo page = ProblemPageParser::parseProblemPage(
                    result.body, result.finalUrl);
                if (m_cache) m_cache->save(m_url.toString(), page);
                emit loaded(page);
            });
}

void ProblemPage::open(const QUrl &url)
{
    m_url = url;
    ProblemPageInfo page;
    m_cacheHit = m_cache && m_cache->load(url.toString(), &page);
    if (m_cacheHit) {
        emit loaded(page);
    }
    emit loadingChanged(true);
    if (m_client) m_client->fetchProblemPage(url);
    else {
        emit loadingChanged(false);
        if (!m_cacheHit) emit failed(QStringLiteral("OpenJudge client is unavailable."));
    }
}
}
