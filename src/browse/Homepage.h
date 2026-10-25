#pragma once

#include <QObject>
#include <QString>

class HomepageCache;
class OpenJudgeClient;

namespace Browse
{
class Homepage : public QObject
{
    Q_OBJECT
public:
    Homepage(OpenJudgeClient *client, HomepageCache *cache, QObject *parent = nullptr);
    void open();

signals:
    void loaded(const QString &personalHomepageUrl);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client;
    HomepageCache *m_cache;
    bool m_cacheHit = false;
};
}
