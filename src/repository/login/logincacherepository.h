#pragma once

#include <QList>
#include <QString>

struct CachedLoginInfo
{
    QString email;
    QString password;
    QString updatedAt;
};

class LoginCacheRepository
{
public:
    LoginCacheRepository();
    ~LoginCacheRepository();

    bool initialize();
    bool saveLogin(const QString &email, const QString &password);
    bool loadLastLogin(CachedLoginInfo *loginInfo) const;
    bool loadLoginByEmail(const QString &email, CachedLoginInfo *loginInfo) const;
    QList<CachedLoginInfo> loadAllLogins() const;
    QString lastError() const;

private:
    bool ensureReady();
    bool openDatabase();
    bool createTable();

    QString m_connectionName;
    mutable QString m_lastError;
    bool m_initialized = false;
};
