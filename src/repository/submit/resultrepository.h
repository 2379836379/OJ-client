#pragma once

#include "parser/resultparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
struct NetworkResult;

class ResultRepository : public QObject
{
    Q_OBJECT

public:
    explicit ResultRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchResultPage(const QUrl &resultPageUrl);

signals:
    void resultPageFetched(const ResultPageInfo &resultPageInfo);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedResultPageUrl;
};
