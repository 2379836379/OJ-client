#include "Group.h"

#include "GroupHomepage.h"
#include "GroupPage.h"

namespace Browse
{
Group::Group(OpenJudgeClient *client,
             GroupHomepageCache *homepageCache,
             GroupPageCache *pageCache,
             QObject *parent)
    : QObject(parent)
    , m_homepage(new GroupHomepage(client, homepageCache, this))
    , m_page(new GroupPage(client, pageCache, this))
{
    connect(m_homepage, &GroupHomepage::loaded, this, [this](const QString &url) {
        m_groupPageUrl = m_url.resolved(QUrl(url));
        m_page->open(m_groupPageUrl);
    });
    connect(m_homepage, &GroupHomepage::failed, this, [this](const QString &message) {
        emit loadingChanged(false);
        emit failed(message);
    });
    connect(m_page, &GroupPage::loaded, this, [this](const GroupPageInfo &page) {
        emit loadingChanged(false);
        emit loaded(m_groupPageUrl, page);
    });
    connect(m_page, &GroupPage::failed, this, [this](const QString &message) {
        emit loadingChanged(false);
        emit failed(message);
    });
}

void Group::open(const QUrl &url)
{
    m_url = url;
    m_groupPageUrl = QUrl();
    emit loadingChanged(true);
    m_homepage->open(url);
}
}
