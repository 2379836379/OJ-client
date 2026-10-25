#pragma once

#include "parser/ProblemPage.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
class ProblemPageCache;

namespace Browse
{
class ProblemPage : public QObject
{
    Q_OBJECT
public:
    ProblemPage(OpenJudgeClient *client, ProblemPageCache *cache,
                QObject *parent = nullptr);
    void open(const QUrl &url);
    QUrl currentUrl() const { return m_url; }

signals:
    void loadingChanged(bool loading);
    void loaded(const ProblemPageInfo &page);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    ProblemPageCache *m_cache;
    QUrl m_url;
    bool m_cacheHit = false;
};
}
