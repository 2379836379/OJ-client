#include "ContestPageCache.h"

#include "PageCacheStore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ContestPageCache::ContestPageCache()
    : m_store(new PageCacheStore("contest_page_cache.db", "contest_page_cache"))
{
}

ContestPageCache::~ContestPageCache() { delete m_store; }

bool ContestPageCache::load(const QString &contestPageUrl, ContestPageInfo *pageInfo)
{
    QByteArray payload;
    if (!m_store->load(contestPageUrl, &payload)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isArray()) return false;
    ContestPageInfo loaded;
    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) return false;
        const QJsonObject object = value.toObject();
        const QJsonValue accepted = object.value("accept_prople");
        const QJsonValue submissions = object.value("submission_people");
        if (!object.value("problemId").isString() || !object.value("title").isString()
            || !object.value("problemUrl").isString() || !accepted.isDouble()
            || !submissions.isDouble() || !object.value("solved").isBool()) return false;
        ContestProblemInfo problem;
        problem.problemId = object.value("problemId").toString();
        problem.title = object.value("title").toString();
        problem.problemUrl = object.value("problemUrl").toString();
        problem.accept_prople = accepted.toInt();
        problem.submission_people = submissions.toInt();
        problem.solved = object.value("solved").toBool();
        loaded.problems.append(problem);
        ++loaded.totalProblems;
        if (problem.solved) ++loaded.solvedProblems;
    }
    if (pageInfo != nullptr) *pageInfo = loaded;
    return true;
}

bool ContestPageCache::save(const QString &contestPageUrl, const ContestPageInfo &pageInfo)
{
    QJsonArray array;
    for (const ContestProblemInfo &problem : pageInfo.problems) {
        array.append(QJsonObject{{"problemId", problem.problemId}, {"title", problem.title},
                                 {"problemUrl", problem.problemUrl},
                                 {"accept_prople", problem.accept_prople},
                                 {"submission_people", problem.submission_people},
                                 {"solved", problem.solved}});
    }
    return m_store->save(contestPageUrl,
                         QJsonDocument(array).toJson(QJsonDocument::Compact));
}

qint64 ContestPageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool ContestPageCache::clearCache() { return m_store->clear(); }
QString ContestPageCache::lastError() const { return m_store->lastError(); }
