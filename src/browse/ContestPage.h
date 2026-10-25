#pragma once

#include "parser/ContestPage.h"

#include <QObject>
#include <QUrl>

class ContestPageCache;
class OpenJudgeClient;

namespace Browse
{
class ContestPage : public QObject
{
    Q_OBJECT
public:
    ContestPage(OpenJudgeClient *client, ContestPageCache *cache,
                QObject *parent = nullptr);
    void open(const QUrl &url);
    QUrl currentUrl() const { return m_url; }

signals:
    void loadingChanged(bool loading);
    void loaded(const ContestPageInfo &page);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    ContestPageCache *m_cache;
    QUrl m_url;
    bool m_cacheHit = false;
};
}
