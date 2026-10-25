#pragma once

#include "parser/ResultPage.h"

#include <QObject>
#include <QTimer>
#include <QUrl>

class OpenJudgeClient;

class ResultService : public QObject
{
    Q_OBJECT

public:
    explicit ResultService(OpenJudgeClient *client, QObject *parent = nullptr);

    void openResult(const QUrl &resultPageUrl);
    QUrl currentResultUrl() const;

signals:
    void loadingChanged(bool loading);
    void resultLoaded(const ResultPageInfo &resultPageInfo);
    void failed(const QString &message);

private:
    void scheduleRetry();

    OpenJudgeClient *m_client = nullptr;
    QUrl m_currentResultUrl;
    QUrl m_expectedResultUrl;
    QTimer m_retryTimer;
};
