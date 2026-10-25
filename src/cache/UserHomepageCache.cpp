#include "UserHomepageCache.h"

#include "PageCacheStore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

UserHomepageCache::UserHomepageCache()
    : m_store(new PageCacheStore("user_homepage_cache.db", "user_homepage_cache"))
{
}

UserHomepageCache::~UserHomepageCache() { delete m_store; }

bool UserHomepageCache::load(const QString &userHomepageUrl,
                             QList<JoinedClassInfo> *classes)
{
    QByteArray payload;
    if (!m_store->load(userHomepageUrl, &payload)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isArray()) return false;
    QList<JoinedClassInfo> loaded;
    for (const QJsonValue &value : doc.array()) {
        if (!value.isObject()) return false;
        const QJsonObject object = value.toObject();
        if (!object.value("name").isString() || !object.value("url").isString()) return false;
        loaded.append({object.value("name").toString(), object.value("url").toString()});
    }
    if (classes != nullptr) *classes = loaded;
    return true;
}

bool UserHomepageCache::save(const QString &userHomepageUrl,
                             const QList<JoinedClassInfo> &classes)
{
    QJsonArray array;
    for (const JoinedClassInfo &item : classes)
        array.append(QJsonObject{{"name", item.name}, {"url", item.url}});
    return m_store->save(userHomepageUrl,
                         QJsonDocument(array).toJson(QJsonDocument::Compact));
}

qint64 UserHomepageCache::cacheSizeBytes() const { return m_store->sizeBytes(); }
bool UserHomepageCache::clearCache() { return m_store->clear(); }
QString UserHomepageCache::lastError() const { return m_store->lastError(); }
