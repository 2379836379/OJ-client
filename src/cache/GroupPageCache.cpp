#include "GroupPageCache.h"

#include "PageCacheStore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

GroupPageCache::GroupPageCache()
    : m_store(new PageCacheStore("group_page_cache.db", "group_page_cache"))
{
}

GroupPageCache::~GroupPageCache() { delete m_store; }

bool GroupPageCache::load(const QString &groupPageUrl, GroupPageInfo *pageInfo)
{
    QByteArray payload;
    if (!m_store->load(groupPageUrl, &payload)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isArray()) return false;
    GroupPageInfo loaded;
    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) return false;
        const QJsonObject object = value.toObject();
        const QJsonValue number = object.value("problemnumber");
        if (!object.value("url").isString() || !object.value("title").isString()
            || !number.isDouble() || !object.value("endtime").isString()) return false;
        loaded.contestSets.append({object.value("url").toString(),
                                   object.value("title").toString(),
                                   number.toInt(),
                                   object.value("endtime").toString()});
    }
    if (pageInfo != nullptr) *pageInfo = loaded;
    return true;
}

bool GroupPageCache::save(const QString &groupPageUrl, const GroupPageInfo &pageInfo)
{
    QJsonArray array;
    for (const ContestSetInfo &item : pageInfo.contestSets) {
        array.append(QJsonObject{{"url", item.url}, {"title", item.title},
                                 {"problemnumber", item.problemnumber},
                                 {"endtime", item.endtime}});
    }
    return m_store->save(groupPageUrl,
                         QJsonDocument(array).toJson(QJsonDocument::Compact));
}

qint64 GroupPageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool GroupPageCache::clearCache() { return m_store->clear(); }
QString GroupPageCache::lastError() const { return m_store->lastError(); }
