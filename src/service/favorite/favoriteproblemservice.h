#pragma once

#include "parser/problemparser.h"

#include <QObject>

class FavoriteProblemRepository;

struct FavoriteFolderInfo
{
    qint64 id = -1;
    QString name;
    int problemCount = 0;
};

class FavoriteProblemService : public QObject
{
    Q_OBJECT

public:
    explicit FavoriteProblemService(FavoriteProblemRepository *repository,
                                    QObject *parent = nullptr);

    bool initialize();
    bool createFolder(const QString &folderName, qint64 *folderId = nullptr);
    bool removeFolder(qint64 folderId);
    bool saveFavoriteToFolder(const ProblemPageInfo &problemPageInfo, qint64 folderId);
    bool removeFavoriteFromFolder(const QString &problemUrl, qint64 folderId);
    QList<FavoriteFolderInfo> loadFolders() const;
    QList<ProblemPageInfo> loadFavoritesInFolder(qint64 folderId) const;
    void refreshFolders();
    void refreshFavoritesInFolder(qint64 folderId);
    bool tryLoadFavorite(const QString &problemUrl,
                         ProblemPageInfo *problemPageInfo = nullptr);
    QString lastError() const;

signals:
    void foldersLoaded(const QList<FavoriteFolderInfo> &folders);
    void folderCreated(const FavoriteFolderInfo &folderInfo);
    void folderRemoved(qint64 folderId);
    void folderFavoritesLoaded(qint64 folderId,
                               const QList<ProblemPageInfo> &favorites);
    void favoriteLoaded(const ProblemPageInfo &problemPageInfo);
    void favoriteSavedToFolder(qint64 folderId,
                               const ProblemPageInfo &problemPageInfo);
    void favoriteRemovedFromFolder(qint64 folderId, const QString &problemUrl);
    void failed(const QString &message);

private:
    FavoriteProblemRepository *m_repository = nullptr;
};
