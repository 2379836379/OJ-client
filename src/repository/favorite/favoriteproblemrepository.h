#pragma once

#include "parser/problemparser.h"
#include "service/favorite/favoriteproblemservice.h"

#include <QList>
#include <QString>

struct ProblemTranslationInfo
{
    QString description;
    QString inputSpec;
    QString outputSpec;
    QString hint;
};

class FavoriteProblemRepository
{
public:
    FavoriteProblemRepository();
    ~FavoriteProblemRepository();

    bool initialize();
    bool createFolder(const QString &folderName, qint64 *folderId = nullptr);
    bool removeFolder(qint64 folderId);
    QList<FavoriteFolderInfo> loadFolders() const;
    QList<ProblemPageInfo> loadFavoritesInFolder(qint64 folderId) const;
    bool saveFavoriteToFolder(const ProblemPageInfo &problemPageInfo, qint64 folderId);
    bool removeFavoriteFromFolder(const QString &problemUrl, qint64 folderId);
    bool loadFavorite(const QString &problemUrl, ProblemPageInfo *problemPageInfo) const;
    bool saveProblemTranslation(const QString &problemUrl,
                                const ProblemTranslationInfo &translationInfo);
    bool loadProblemTranslation(const QString &problemUrl,
                                ProblemTranslationInfo *translationInfo) const;
    QString lastError() const;

private:
    bool openDatabase();
    bool ensureSchema();
    bool upsertProblem(const ProblemPageInfo &problemPageInfo);
    ProblemPageInfo readProblem(const class QSqlQuery &query) const;

    QString m_connectionName;
    QString m_lastError;
};
