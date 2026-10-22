#pragma once

#include "parser/problemparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;
struct NetworkResult;

class ProblemRepository : public QObject
{
    Q_OBJECT

public:
    explicit ProblemRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchProblemDetail(const QUrl &problemUrl);

signals:
    void problemFetched(const ProblemPageInfo &problemPageInfo);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedProblemUrl;
};
