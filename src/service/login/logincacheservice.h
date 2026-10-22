#pragma once

#include "repository/login/logincacherepository.h"

#include <QObject>

class LoginCacheRepository;

class LoginCacheService : public QObject
{
    Q_OBJECT

public:
    explicit LoginCacheService(LoginCacheRepository *repository,
                               QObject *parent = nullptr);

    bool initialize();
    bool saveLogin(const QString &email, const QString &password);
    bool loadLastLogin(CachedLoginInfo *loginInfo) const;
    bool tryLoadLoginByEmail(const QString &email,
                             CachedLoginInfo *loginInfo = nullptr) const;
    QList<CachedLoginInfo> loadAllLogins() const;
    QString lastError() const;

signals:
    void failed(const QString &message);

private:
    LoginCacheRepository *m_repository = nullptr;
};
