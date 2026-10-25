#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class GroupHomepageCache;
class OpenJudgeClient;

namespace Browse
{
class GroupHomepage : public QObject
{
    Q_OBJECT
public:
    GroupHomepage(OpenJudgeClient *client, GroupHomepageCache *cache,
                  QObject *parent = nullptr);
    void open(const QUrl &url);
    QUrl currentUrl() const { return m_url; }

signals:
    void loadingChanged(bool loading);
    void loaded(const QString &groupUrl);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    GroupHomepageCache *m_cache;
    QUrl m_url;
    bool m_cacheHit = false;
};
}
