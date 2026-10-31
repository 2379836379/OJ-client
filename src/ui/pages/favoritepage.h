#pragma once

#include "favorite/favoriteproblemservice.h"
#include "parser/ProblemPage.h"

#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QLabel;
class QPushButton;
class PageShell;

class FavoritePage : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritePage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showFoldersUnavailable();
    void showFolders(const QList<FavoriteFolderInfo> &folders);
    void showRefreshUnavailable();
    void showFavoriteOperationFailed(const QString &message);
    void showFavorites(qint64 folderId,
                       const QString &folderName,
                       const QList<ProblemPageInfo> &favorites);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void exportRequested();
    void importRequested();
    void createFolderRequested(const QString &folderName);
    void folderRemoveRequested(qint64 folderId, const QString &folderName);
    void folderSelected(qint64 folderId, const QString &folderName);
    void favoriteSelected(const QString &title, const QString &url);
    void favoriteRemoveRequested(qint64 folderId,
                                 const QString &folderName,
                                 const QString &problemUrl);

private:
    enum class ViewMode
    {
        FolderList,
        FavoriteList
    };

    void setViewMode(ViewMode mode, const QString &folderName = QString());
    void updateToolVisibility();
    void removeSelectedFolder();
    void removeSelectedFavorite();

    PageShell *m_shell = nullptr;
    QLabel *m_sectionLabel = nullptr;
    QListWidget *m_favoriteListWidget = nullptr;
    QPushButton *m_newFolderTool = nullptr;
    QPushButton *m_collapsedNewFolderTool = nullptr;
    QPushButton *m_removeFolderTool = nullptr;
    QPushButton *m_collapsedRemoveFolderTool = nullptr;
    QPushButton *m_removeFavoriteTool = nullptr;
    QPushButton *m_collapsedRemoveFavoriteTool = nullptr;
    ViewMode m_viewMode = ViewMode::FolderList;
    qint64 m_currentFolderId = -1;
    QString m_currentFolderName;
    bool m_darkMode = false;
};
