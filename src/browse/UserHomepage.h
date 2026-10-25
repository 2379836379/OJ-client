#pragma once

#include "parser/UserHomepage.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
class UserHomepageCache;

namespace Browse
{
class UserHomepage : public QObject
{
    Q_OBJECT
public:
    UserHomepage(OpenJudgeClient *client, UserHomepageCache *cache,
                 QObject *parent = nullptr);
    void open(const QUrl &url);
    QUrl currentUrl() const { return m_stateUrl; }

signals:
    void loadingChanged(bool loading);
    void loaded(const QList<JoinedClassInfo> &classes);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    UserHomepageCache *m_cache;
    QUrl m_stateUrl;
    bool m_cacheHit = false;
};
}
