#pragma once

#include "config/apppaths.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

class PageCacheStore
{
public:
    PageCacheStore(const QString &fileName, const QString &tableName)
        : m_path(QDir(AppPaths::cacheDir()).filePath(fileName))
        , m_tableName(tableName)
        , m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces))
    {
    }

    ~PageCacheStore()
    {
        closeDatabase();
    }

    bool load(const QString &key, QByteArray *payload)
    {
        if (!ensureReady()) {
            return false;
        }
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(QString("SELECT payload FROM %1 WHERE cache_key = :key")
                          .arg(m_tableName));
        query.bindValue(":key", key);
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            return false;
        }
        if (!query.next()) {
            return false;
        }
        if (payload != nullptr) {
            *payload = query.value(0).toByteArray();
        }
        return true;
    }

    bool save(const QString &key, const QByteArray &payload)
    {
        if (!ensureReady()) {
            return false;
        }
        QSqlQuery query(QSqlDatabase::database(m_connectionName));
        query.prepare(QString(
                          "INSERT INTO %1 (cache_key, payload, cached_at) "
                          "VALUES (:key, :payload, CURRENT_TIMESTAMP) "
                          "ON CONFLICT(cache_key) DO UPDATE SET "
                          "payload = excluded.payload, cached_at = CURRENT_TIMESTAMP")
                          .arg(m_tableName));
        query.bindValue(":key", key);
        query.bindValue(":payload", payload);
        if (!query.exec()) {
            m_lastError = query.lastError().text();
            return false;
        }
        return true;
    }

    qint64 sizeBytes() const
    {
        qint64 total = 0;
        for (const QString &suffix : {QString(), QString("-wal"), QString("-shm")}) {
            const QFileInfo info(m_path + suffix);
            if (info.exists()) {
                total += info.size();
            }
        }
        return total;
    }

    bool clear()
    {
        closeDatabase();
        bool ok = true;
        for (const QString &suffix : {QString(), QString("-wal"), QString("-shm")}) {
            const QString path = m_path + suffix;
            if (QFile::exists(path) && !QFile::remove(path)) {
                ok = false;
            }
        }
        if (!ok) {
            m_lastError = QString("Failed to remove cache file: %1").arg(m_path);
        }
        return ok;
    }

    QString lastError() const { return m_lastError; }

private:
    bool ensureReady()
    {
        if (m_initialized) {
            return true;
        }
        if (!QDir().mkpath(QFileInfo(m_path).absolutePath())) {
            m_lastError = QString("Failed to create cache directory for %1").arg(m_path);
            return false;
        }
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        database.setDatabaseName(m_path);
        if (!database.open()) {
            m_lastError = database.lastError().text();
            return false;
        }
        QSqlQuery query(database);
        if (!query.exec(QString(
                            "CREATE TABLE IF NOT EXISTS %1 ("
                            "cache_key TEXT PRIMARY KEY, payload BLOB NOT NULL, "
                            "cached_at TEXT DEFAULT CURRENT_TIMESTAMP)")
                            .arg(m_tableName))) {
            m_lastError = query.lastError().text();
            return false;
        }
        m_initialized = true;
        return true;
    }

    void closeDatabase()
    {
        if (QSqlDatabase::contains(m_connectionName)) {
            {
                QSqlDatabase database = QSqlDatabase::database(m_connectionName);
                database.close();
            }
            QSqlDatabase::removeDatabase(m_connectionName);
        }
        m_initialized = false;
    }

    QString m_path;
    QString m_tableName;
    QString m_connectionName;
    QString m_lastError;
    bool m_initialized = false;
};
