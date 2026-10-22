#pragma once

#include "parser/classparser.h"
#include "parser/groupparser.h"

#include <QObject>
#include <QUrl>

class ClassRepository;
class ClassCacheRepository;

class ClassService : public QObject
{
    Q_OBJECT

public:
    explicit ClassService(ClassRepository *repository,
                          ClassCacheRepository *cacheRepository,
                          QObject *parent = nullptr);

    void openClass(const QUrl &classPageUrl);
    QUrl currentClassUrl() const;

signals:
    void loadingChanged(bool loading);
    void classLoaded(const ClassPageInfo &classPageInfo,
                     const GroupPageInfo &groupPageInfo);
    void failed(const QString &message);

private:
    ClassRepository *m_repository = nullptr;
    ClassCacheRepository *m_cacheRepository = nullptr;
    QUrl m_currentClassUrl;
    bool m_loading = false;
    ClassPageInfo m_lastClassInfo;
    GroupPageInfo m_lastGroupInfo;
    bool m_hasLastLoadedData = false;
    bool m_showingCachedData = false;
};
