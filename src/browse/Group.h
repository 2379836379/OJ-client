#pragma once

#include "parser/GroupPage.h"

#include <QObject>
#include <QUrl>

class GroupHomepageCache;
class GroupPageCache;
class OpenJudgeClient;

namespace Browse
{
class GroupHomepage;
class GroupPage;

class Group : public QObject
{
    Q_OBJECT
public:
    Group(OpenJudgeClient *client,
          GroupHomepageCache *homepageCache,
          GroupPageCache *pageCache,
          QObject *parent = nullptr);

    void open(const QUrl &url);
    QUrl currentUrl() const { return m_url; }

signals:
    void loadingChanged(bool loading);
    void loaded(const QUrl &groupPageUrl, const GroupPageInfo &page);
    void failed(const QString &message);

private:
    GroupHomepage *m_homepage;
    GroupPage *m_page;
    QUrl m_url;
    QUrl m_groupPageUrl;
};
}
