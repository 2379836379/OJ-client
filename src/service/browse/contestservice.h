#pragma once

#include "parser/contestparser.h"

#include <QObject>
#include <QUrl>

class ContestRepository;
class ContestCacheRepository;

class ContestService : public QObject
{
    Q_OBJECT

public:
    explicit ContestService(ContestRepository *repository,
                            ContestCacheRepository *cacheRepository,
                            QObject *parent = nullptr);

    void openContest(const QUrl &contestPageUrl);
    QUrl currentContestUrl() const;

signals:
    void loadingChanged(bool loading);
    void contestLoaded(const ContestPageInfo &contestPageInfo);
    void failed(const QString &message);

private:
    bool sameContestPage(const ContestPageInfo &lhs,
                         const ContestPageInfo &rhs) const;

    ContestRepository *m_repository = nullptr;
    ContestCacheRepository *m_cacheRepository = nullptr;
    QUrl m_currentContestUrl;
    bool m_loading = false;
    ContestPageInfo m_lastContestInfo;
    bool m_hasLastContestInfo = false;
};
