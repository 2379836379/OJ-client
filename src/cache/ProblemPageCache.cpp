#include "ProblemPageCache.h"

#include "PageCacheStore.h"

#include <QJsonDocument>
#include <QJsonObject>

ProblemPageCache::ProblemPageCache()
    : m_store(new PageCacheStore("problem_page_cache.db", "problem_page_cache"))
{
}

ProblemPageCache::~ProblemPageCache() { delete m_store; }

bool ProblemPageCache::load(const QString &problemPageUrl, ProblemPageInfo *pageInfo)
{
    QByteArray payload;
    if (!m_store->load(problemPageUrl, &payload)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) return false;
    const QJsonObject object = doc.object();
    const QStringList keys{"submitUrl", "timeLimit", "memoryLimit", "description",
                           "starterCode", "inputSpec", "outputSpec", "sampleInput",
                           "sampleOutput", "hint"};
    for (const QString &key : keys) if (!object.value(key).isString()) return false;
    ProblemPageInfo loaded;
    loaded.submitUrl = object.value("submitUrl").toString();
    loaded.timeLimit = object.value("timeLimit").toString();
    loaded.memoryLimit = object.value("memoryLimit").toString();
    loaded.description = object.value("description").toString();
    loaded.starterCode = object.value("starterCode").toString();
    loaded.inputSpec = object.value("inputSpec").toString();
    loaded.outputSpec = object.value("outputSpec").toString();
    loaded.sampleInput = object.value("sampleInput").toString();
    loaded.sampleOutput = object.value("sampleOutput").toString();
    loaded.hint = object.value("hint").toString();
    if (pageInfo != nullptr) *pageInfo = loaded;
    return true;
}

bool ProblemPageCache::save(const QString &problemPageUrl, const ProblemPageInfo &pageInfo)
{
    const QJsonObject object{{"submitUrl", pageInfo.submitUrl}, {"timeLimit", pageInfo.timeLimit},
                             {"memoryLimit", pageInfo.memoryLimit}, {"description", pageInfo.description},
                             {"starterCode", pageInfo.starterCode}, {"inputSpec", pageInfo.inputSpec},
                             {"outputSpec", pageInfo.outputSpec}, {"sampleInput", pageInfo.sampleInput},
                             {"sampleOutput", pageInfo.sampleOutput}, {"hint", pageInfo.hint}};
    return m_store->save(problemPageUrl,
                         QJsonDocument(object).toJson(QJsonDocument::Compact));
}

qint64 ProblemPageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool ProblemPageCache::clearCache() { return m_store->clear(); }
QString ProblemPageCache::lastError() const { return m_store->lastError(); }
