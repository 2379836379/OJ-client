#include "service/login/logincacheservice.h"

LoginCacheService::LoginCacheService(LoginCacheRepository *repository,
                                     QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
}

bool LoginCacheService::initialize()
{
    if (m_repository == nullptr) {
        emit failed("Login cache repository not available");
        return false;
    }

    const bool ok = m_repository->initialize();
    if (!ok) {
        emit failed(m_repository->lastError());
    }
    return ok;
}

bool LoginCacheService::saveLogin(const QString &email, const QString &password)
{
    if (m_repository == nullptr) {
        emit failed("Login cache repository not available");
        return false;
    }

    const bool ok = m_repository->saveLogin(email, password);
    if (!ok) {
        emit failed(m_repository->lastError());
    }
    return ok;
}

bool LoginCacheService::loadLastLogin(CachedLoginInfo *loginInfo) const
{
    return m_repository != nullptr && m_repository->loadLastLogin(loginInfo);
}

bool LoginCacheService::tryLoadLoginByEmail(const QString &email,
                                            CachedLoginInfo *loginInfo) const
{
    return m_repository != nullptr
           && m_repository->loadLoginByEmail(email, loginInfo);
}

QList<CachedLoginInfo> LoginCacheService::loadAllLogins() const
{
    return m_repository == nullptr ? QList<CachedLoginInfo>()
                                   : m_repository->loadAllLogins();
}

QString LoginCacheService::lastError() const
{
    return m_repository == nullptr ? QString("Login cache repository not available")
                                   : m_repository->lastError();
}
