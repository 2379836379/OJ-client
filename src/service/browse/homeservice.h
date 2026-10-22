#pragma once

#include "parser/loginparser.h"

#include <QObject>
#include <QUrl>

class HomeRepository;
class HomeCacheRepository;

class HomeService : public QObject
{
    Q_OBJECT

public:
    explicit HomeService(HomeRepository *repository,
                         HomeCacheRepository *cacheRepository,
                         QObject *parent = nullptr);

    void openHome(const QUrl &userHomeUrl);
    QUrl currentHomeUrl() const;

signals:
    void loadingChanged(bool loading);
    void homeLoaded(const QList<JoinedClassInfo> &classes);
    void failed(const QString &message);

private:
    HomeRepository *m_repository = nullptr;
    HomeCacheRepository *m_cacheRepository = nullptr;
    QUrl m_currentHomeUrl;
    bool m_loading = false;
    QList<JoinedClassInfo> m_lastLoadedClasses;
    bool m_hasLastLoadedClasses = false;
    bool m_showingCachedData = false;
};
