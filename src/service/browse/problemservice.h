#pragma once

#include "parser/problemparser.h"

#include <QObject>
#include <QUrl>

class ProblemRepository;
class ProblemCacheRepository;

class ProblemService : public QObject
{
    Q_OBJECT

public:
    explicit ProblemService(ProblemRepository *repository,
                            ProblemCacheRepository *cacheRepository,
                            QObject *parent = nullptr);

    void openProblem(const QUrl &problemUrl);
    QUrl currentProblemUrl() const;

signals:
    void loadingChanged(bool loading);
    void problemLoaded(const ProblemPageInfo &problemPageInfo);
    void failed(const QString &message);

private:
    bool sameProblemPage(const ProblemPageInfo &lhs,
                         const ProblemPageInfo &rhs) const;

    ProblemRepository *m_repository = nullptr;
    ProblemCacheRepository *m_cacheRepository = nullptr;
    QUrl m_currentProblemUrl;
    bool m_loading = false;
    ProblemPageInfo m_lastProblemInfo;
    bool m_hasLastProblemInfo = false;
    bool m_showingCachedData = false;
};
