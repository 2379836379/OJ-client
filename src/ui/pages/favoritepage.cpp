#include "ui/pages/favoritepage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

FavoritePage::FavoritePage(QWidget *parent)
    : QWidget(parent)
{
    const auto handleBack = [this]() {
        if (m_viewMode == ViewMode::FavoriteList) {
            m_favoriteListWidget->clear();
            m_currentFolderId = -1;
            m_currentFolderName.clear();
            setViewMode(ViewMode::FolderList);
            emit refreshRequested();
            return;
        }
        emit backRequested();
    };

    m_shell = new PageShell(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);
    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_newFolderTool = m_shell->addTool("New Folder", "Create a favorite folder", [this]() {
        const QString folderName = QInputDialog::getText(this, "New Folder", "Folder name:");
        if (!folderName.trimmed().isEmpty()) {
            emit createFolderRequested(folderName.trimmed());
        }
    }, "plus.svg", &m_collapsedNewFolderTool);
    m_removeFolderTool = m_shell->addTool("Delete Folder", "Delete the selected favorite folder",
                                          [this]() { removeSelectedFolder(); },
                                          "minus.svg", &m_collapsedRemoveFolderTool);
    m_removeFavoriteTool = m_shell->addTool("Remove Problem", "Remove the selected favorite problem",
                                            [this]() { removeSelectedFavorite(); },
                                            "trash.svg", &m_collapsedRemoveFavoriteTool);
    m_shell->addTool("Back", "Back", handleBack, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *contentLayout = m_shell->contentLayout();
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(14);
    auto *titleLabel = new QLabel("Favorites", m_shell);
    titleLabel->setObjectName("favoriteTitleLabel");
    contentLayout->addWidget(titleLabel);

    m_favoriteListWidget = new QListWidget(m_shell);
    m_favoriteListWidget->setObjectName("favoriteListWidget");
    m_favoriteListWidget->setMinimumHeight(420);

    contentLayout->addWidget(m_favoriteListWidget, 1);

    m_shell->setDarkMode(false);

    setStyleSheet(
        "#favoriteTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #000000;"
        "}"
        "#favoriteListWidget {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0px;"
        "  padding: 0px;"
        "  outline: none;"
        "}"
        "#favoriteListWidget::item {"
        "  padding: 12px 0px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#favoriteListWidget::item:selected {"
        "  background: #CCE5FF;"
        "  color: #000000;"
        "}"
        "#favoriteListWidget::item:hover {"
        "  background: #E8E8E8;"
        "}"
    );
    updateToolVisibility();

    connect(
        m_favoriteListWidget,
        &QListWidget::itemClicked,
        this,
        [this](QListWidgetItem *item) {
            if (m_viewMode == ViewMode::FolderList) {
                emit folderSelected(
                    item->data(Qt::UserRole).toLongLong(),
                    item->toolTip().isEmpty() ? item->text() : item->toolTip());
                return;
            }

            emit favoriteSelected(item->text(), item->data(Qt::UserRole).toString());
        });
    connect(
        m_favoriteListWidget,
        &QListWidget::currentItemChanged,
        this,
        [this](QListWidgetItem *current, QListWidgetItem *) {
            m_removeFolderTool->setEnabled(
                m_viewMode == ViewMode::FolderList && current != nullptr);
            m_collapsedRemoveFolderTool->setEnabled(
                m_viewMode == ViewMode::FolderList && current != nullptr);
            m_removeFavoriteTool->setEnabled(
                m_viewMode == ViewMode::FavoriteList && current != nullptr);
            m_collapsedRemoveFavoriteTool->setEnabled(
                m_viewMode == ViewMode::FavoriteList && current != nullptr);
        });
}

void FavoritePage::showFoldersUnavailable()
{
    m_favoriteListWidget->clear();
    setViewMode(ViewMode::FolderList);
    updateToolVisibility();
}

void FavoritePage::showFolders(const QList<FavoriteFolderInfo> &folders)
{
    m_favoriteListWidget->clear();
    setViewMode(ViewMode::FolderList);
    updateToolVisibility();
    if (folders.isEmpty()) {
        return;
    }

    for (const FavoriteFolderInfo &folder : folders) {
        const QString displayText = QString("%1").arg(folder.name);
        auto *item = new QListWidgetItem(displayText, m_favoriteListWidget);
        item->setData(Qt::UserRole, folder.id);
        item->setToolTip(folder.name);
    }
}

void FavoritePage::showRefreshUnavailable()
{
}

void FavoritePage::showFavoriteOperationFailed(const QString &message)
{
}

void FavoritePage::showFavorites(qint64 folderId,
                                 const QString &folderName,
                                 const QList<ProblemPageInfo> &favorites)
{
    m_favoriteListWidget->clear();
    m_currentFolderId = folderId;
    m_currentFolderName = folderName;
    setViewMode(ViewMode::FavoriteList, folderName);
    updateToolVisibility();
    if (favorites.isEmpty()) {
        return;
    }

    for (const ProblemPageInfo &favorite : favorites) {
        const QString displayText = favorite.title.isEmpty()
            ? favorite.problemUrl
            : favorite.title;
        auto *item = new QListWidgetItem(displayText, m_favoriteListWidget);
        item->setData(Qt::UserRole, favorite.problemUrl);
    }
}

void FavoritePage::setViewMode(ViewMode mode, const QString &folderName)
{
    m_viewMode = mode;  
    updateToolVisibility();
}

void FavoritePage::updateToolVisibility()
{
    const bool folderMode = m_viewMode == ViewMode::FolderList;
    const bool hasSelection = m_favoriteListWidget->currentItem() != nullptr;
    m_newFolderTool->setVisible(folderMode);
    m_collapsedNewFolderTool->setVisible(folderMode);
    m_removeFolderTool->setVisible(folderMode);
    m_collapsedRemoveFolderTool->setVisible(folderMode);
    m_removeFavoriteTool->setVisible(!folderMode);
    m_collapsedRemoveFavoriteTool->setVisible(!folderMode);
    m_removeFolderTool->setEnabled(folderMode && hasSelection);
    m_collapsedRemoveFolderTool->setEnabled(folderMode && hasSelection);
    m_removeFavoriteTool->setEnabled(!folderMode && hasSelection);
    m_collapsedRemoveFavoriteTool->setEnabled(!folderMode && hasSelection);
}

void FavoritePage::removeSelectedFolder()
{
    QListWidgetItem *item = m_favoriteListWidget->currentItem();
    if (m_viewMode != ViewMode::FolderList || item == nullptr) {
        return;
    }
    emit folderRemoveRequested(item->data(Qt::UserRole).toLongLong(),
                               item->toolTip().isEmpty() ? item->text() : item->toolTip());
}

void FavoritePage::removeSelectedFavorite()
{
    QListWidgetItem *item = m_favoriteListWidget->currentItem();
    if (m_viewMode != ViewMode::FavoriteList || m_currentFolderId < 0 || item == nullptr) {
        return;
    }
    emit favoriteRemoveRequested(m_currentFolderId,
                                 m_currentFolderName,
                                 item->data(Qt::UserRole).toString());
}

void FavoritePage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "#favoriteTitleLabel {"
        "  color: #FFFFFF;"
        "}"
        "#favoriteListWidget { color: #FFFFFF; }"
        "#favoriteListWidget::item:selected {"
        "  background: #007ACC;"
        "  color: #FFFFFF;"
        "}"
        "#favoriteListWidget::item:hover {"
        "  background: #2A2D2E;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
