#pragma once

#include "network/openjudgeclient.h"
#include "parser/submitparser.h"

#include <QObject>
#include <QUrl>

class OpenJudgeClient;

class SubmitRepository : public QObject
{
    Q_OBJECT

public:
    explicit SubmitRepository(OpenJudgeClient *client, QObject *parent = nullptr);

    void fetchSubmitPage(const QUrl &submitPageUrl);
    void submitSolution(const QUrl &submitActionUrl,
                        const QByteArray &payload,
                        const QUrl &refererUrl);

signals:
    void submitPageFetched(const SubmitPageInfo &submitPageInfo);
    void solutionSubmitted(const NetworkResult &result);
    void requestFailed(const QString &stage, const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    QUrl m_expectedSubmitPageUrl;
    QUrl m_expectedSubmitActionUrl;
};
