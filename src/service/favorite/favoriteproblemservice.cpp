#include "favoriteproblemservice.h"

#include "repository/favorite/favoriteproblemrepository.h"

FavoriteProblemService::FavoriteProblemService(
    FavoriteProblemRepository *repository, QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
}

bool FavoriteProblemService::initialize()
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return false;
    }

    const bool ok = m_repository->initialize();
    if (!ok) {
        emit failed(m_repository->lastError());
        return false;
    }

    refreshFolders();
    return true;
}

bool FavoriteProblemService::createFolder(const QString &folderName, qint64 *folderId)
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return false;
    }

    qint64 createdFolderId = -1;
    const bool ok = m_repository->createFolder(folderName, &createdFolderId);
    if (!ok) {
        emit failed(m_repository->lastError());
        return false;
    }

    if (folderId != nullptr) {
        *folderId = createdFolderId;
    }

    FavoriteFolderInfo createdFolder;
    const QList<FavoriteFolderInfo> folders = m_repository->loadFolders();
    for (const FavoriteFolderInfo &folder : folders) {
        if (folder.id == createdFolderId) {
            createdFolder = folder;
            break;
        }
    }

    emit folderCreated(createdFolder);
    emit foldersLoaded(folders);
    return true;
}

bool FavoriteProblemService::removeFolder(qint64 folderId)
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return false;
    }

    const bool ok = m_repository->removeFolder(folderId);
    if (!ok) {
        emit failed(m_repository->lastError());
        return false;
    }

    emit folderRemoved(folderId);
    emit foldersLoaded(m_repository->loadFolders());
    return true;
}

bool FavoriteProblemService::saveFavoriteToFolder(const ProblemPageInfo &problemPageInfo,
                                                  qint64 folderId)
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return false;
    }

    const bool ok = m_repository->saveFavoriteToFolder(problemPageInfo, folderId);
    if (!ok) {
        emit failed(m_repository->lastError());
        return false;
    }

    emit favoriteSavedToFolder(folderId, problemPageInfo);
    refreshFolders();
    refreshFavoritesInFolder(folderId);
    return true;
}

bool FavoriteProblemService::removeFavoriteFromFolder(const QString &problemUrl,
                                                      qint64 folderId)
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return false;
    }

    const bool ok = m_repository->removeFavoriteFromFolder(problemUrl, folderId);
    if (!ok) {
        emit failed(m_repository->lastError());
        return false;
    }

    emit favoriteRemovedFromFolder(folderId, problemUrl);
    refreshFolders();
    refreshFavoritesInFolder(folderId);
    return true;
}

QList<FavoriteFolderInfo> FavoriteProblemService::loadFolders() const
{
    return m_repository == nullptr ? QList<FavoriteFolderInfo>()
                                   : m_repository->loadFolders();
}

QList<ProblemPageInfo> FavoriteProblemService::loadFavoritesInFolder(qint64 folderId) const
{
    return m_repository == nullptr ? QList<ProblemPageInfo>()
                                   : m_repository->loadFavoritesInFolder(folderId);
}

void FavoriteProblemService::refreshFolders()
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return;
    }

    emit foldersLoaded(m_repository->loadFolders());
}

void FavoriteProblemService::refreshFavoritesInFolder(qint64 folderId)
{
    if (m_repository == nullptr) {
        emit failed("Favorite repository not available");
        return;
    }

    emit folderFavoritesLoaded(
        folderId,
        m_repository->loadFavoritesInFolder(folderId));
}

bool FavoriteProblemService::tryLoadFavorite(
    const QString &problemUrl, ProblemPageInfo *problemPageInfo)
{
    if (m_repository == nullptr) {
        return false;
    }

    ProblemPageInfo loadedProblemInfo;
    if (!m_repository->loadFavorite(problemUrl, &loadedProblemInfo)) {
        return false;
    }

    if (problemPageInfo != nullptr) {
        *problemPageInfo = loadedProblemInfo;
    }
    emit favoriteLoaded(loadedProblemInfo);
    return true;
}

QString FavoriteProblemService::lastError() const
{
    return m_repository == nullptr ? QString("Favorite repository not available")
                                   : m_repository->lastError();
}
