#include "repository/login/logincacherepository.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace
{
QString databasePath()
{
    return QDir(AppPaths::dataDir()).filePath("login_cache.db");
}
}

LoginCacheRepository::LoginCacheRepository()
    : m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

LoginCacheRepository::~LoginCacheRepository()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName);
        database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool LoginCacheRepository::initialize()
{
    return ensureReady();
}

bool LoginCacheRepository::saveLogin(const QString &email, const QString &password)
{
    if (!ensureReady()) {
        return false;
    }

    const QString normalizedEmail = email.trimmed();
    if (normalizedEmail.isEmpty()) {
        m_lastError = "Email cannot be empty";
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO login_cache (email, password, updated_at) "
        "VALUES (:email, :password, CURRENT_TIMESTAMP) "
        "ON CONFLICT(email) DO UPDATE SET "
        "password = excluded.password, "
        "updated_at = CURRENT_TIMESTAMP");
    query.bindValue(":email", normalizedEmail);
    query.bindValue(":password", password);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

bool LoginCacheRepository::loadLastLogin(CachedLoginInfo *loginInfo) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT email, password, updated_at "
            "FROM login_cache "
            "ORDER BY updated_at DESC, email ASC "
            "LIMIT 1")) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    if (loginInfo != nullptr) {
        loginInfo->email = query.value(0).toString();
        loginInfo->password = query.value(1).toString();
        loginInfo->updatedAt = query.value(2).toString();
    }
    return true;
}

bool LoginCacheRepository::loadLoginByEmail(const QString &email,
                                            CachedLoginInfo *loginInfo) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT email, password, updated_at "
        "FROM login_cache "
        "WHERE email = :email");
    query.bindValue(":email", email.trimmed());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    if (loginInfo != nullptr) {
        loginInfo->email = query.value(0).toString();
        loginInfo->password = query.value(1).toString();
        loginInfo->updatedAt = query.value(2).toString();
    }
    return true;
}

QList<CachedLoginInfo> LoginCacheRepository::loadAllLogins() const
{
    QList<CachedLoginInfo> records;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return records;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT email, password, updated_at "
            "FROM login_cache "
            "ORDER BY updated_at DESC, email ASC")) {
        m_lastError = query.lastError().text();
        return records;
    }

    while (query.next()) {
        CachedLoginInfo info;
        info.email = query.value(0).toString();
        info.password = query.value(1).toString();
        info.updatedAt = query.value(2).toString();
        records.append(info);
    }

    return records;
}

QString LoginCacheRepository::lastError() const
{
    return m_lastError;
}

bool LoginCacheRepository::ensureReady()
{
    if (m_initialized) {
        return true;
    }

    if (!openDatabase()) {
        return false;
    }

    if (!createTable()) {
        return false;
    }

    m_initialized = true;
    return true;
}

bool LoginCacheRepository::openDatabase()
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase database =
            QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        database.setDatabaseName(databasePath());
        if (!database.open()) {
            m_lastError = database.lastError().text();
            return false;
        }
        return true;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    if (database.isOpen()) {
        return true;
    }

    const bool ok = database.open();
    if (!ok) {
        m_lastError = database.lastError().text();
    }
    return ok;
}

bool LoginCacheRepository::createTable()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS login_cache ("
        "email TEXT PRIMARY KEY,"
        "password TEXT NOT NULL,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}
