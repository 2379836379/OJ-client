#pragma once

#include "parser/contestparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
struct NetworkResult;

class ContestRepository : public QObject
{
    Q_OBJECT

public:
    explicit ContestRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchProblems(const QUrl &contestPageUrl);

signals:
    void problemsFetched(const ContestPageInfo &contestPageInfo);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedContestPageUrl;
};
