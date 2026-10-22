#pragma once

#include "parser/resultparser.h"

#include <QObject>
#include <QTimer>
#include <QUrl>

class ResultRepository;

class ResultService : public QObject
{
    Q_OBJECT

public:
    explicit ResultService(ResultRepository *repository, QObject *parent = nullptr);

    void openResult(const QUrl &resultPageUrl);
    QUrl currentResultUrl() const;

signals:
    void loadingChanged(bool loading);
    void resultLoaded(const ResultPageInfo &resultPageInfo);
    void failed(const QString &message);

private:
    void scheduleRetry();

    ResultRepository *m_repository = nullptr;
    QUrl m_currentResultUrl;
    QTimer m_retryTimer;
    bool m_loading = false;
};
