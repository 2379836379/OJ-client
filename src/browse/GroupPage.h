#pragma once

#include "parser/GroupPage.h"

#include <QObject>
#include <QUrl>

class GroupPageCache;
class OpenJudgeClient;

namespace Browse
{
class GroupPage : public QObject
{
    Q_OBJECT
public:
    GroupPage(OpenJudgeClient *client, GroupPageCache *cache, QObject *parent = nullptr);
    void open(const QUrl &url);
    QUrl currentUrl() const { return m_url; }

signals:
    void loadingChanged(bool loading);
    void loaded(const GroupPageInfo &page);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    GroupPageCache *m_cache;
    QUrl m_url;
    bool m_cacheHit = false;
};
}
