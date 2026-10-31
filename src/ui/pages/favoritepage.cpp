#include "ui/pages/favoritepage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
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
            m_statusLabel->clear();
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
    m_shell->addTool("Export", "Export all favorite folders to a JSON file",
                     [this]() { emit exportRequested(); });
    m_shell->addTool("Import", "Import favorite folders from a JSON file (merge)",
                     [this]() { emit importRequested(); });
    m_shell->addTool("Back", "Back", handleBack, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *contentLayout = m_shell->contentLayout();
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(14);
    auto *titleLabel = new QLabel("Favorites", m_shell);
    titleLabel->setObjectName("favoriteTitleLabel");
    contentLayout->addWidget(titleLabel);

    m_sectionLabel = new QLabel("Favorite Folders", m_shell);
    m_sectionLabel->setObjectName("favoriteSectionLabel");

    m_statusLabel = new QLabel(m_shell);
    m_statusLabel->setObjectName("favoriteStatusLabel");
    m_statusLabel->setWordWrap(true);

    m_favoriteListWidget = new QListWidget(m_shell);
    m_favoriteListWidget->setObjectName("favoriteListWidget");
    m_favoriteListWidget->setMinimumHeight(420);

    auto *folderActionsLayout = new QHBoxLayout();
    folderActionsLayout->setSpacing(10);
    m_newFolderButton = new QPushButton("New Folder", m_shell);
    m_newFolderButton->setObjectName("favoriteActionButton");
    m_removeFolderButton = new QPushButton("Delete Folder", m_shell);
    m_removeFolderButton->setObjectName("favoriteActionButton");
    m_removeFolderButton->setEnabled(false);
    folderActionsLayout->addWidget(m_newFolderButton);
    folderActionsLayout->addWidget(m_removeFolderButton);
    folderActionsLayout->addStretch();

    m_removeButton = new QPushButton("Remove Selected", m_shell);
    m_removeButton->setObjectName("favoriteActionButton");
    m_removeButton->setVisible(false);
    m_removeButton->setEnabled(false);

    contentLayout->addWidget(m_sectionLabel);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(m_favoriteListWidget, 1);
    contentLayout->addLayout(folderActionsLayout);
    contentLayout->addWidget(m_removeButton, 0, Qt::AlignLeft);

    m_shell->setDarkMode(false);

    setStyleSheet(
        "#favoriteTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#favoriteSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#favoriteActionButton {"
        "  padding: 10px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#favoriteActionButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#favoriteStatusLabel {"
        "  color: #7a4b36;"
        "  min-height: 20px;"
        "}"
        "#favoriteListWidget {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0px;"
        "  padding: 0px;"
        "  outline: none;"
        "}"
        "#favoriteListWidget::item {"
        "  padding: 12px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#favoriteListWidget::item:selected {"
        "  background: #dcefea;"
        "  color: #12343b;"
        "}"
        "#favoriteListWidget::item:hover {"
        "  background: #eef4ef;"
        "}"
    );

    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &FavoritePage::refreshRequested);
    connect(
        m_newFolderButton,
        &QPushButton::clicked,
        this,
        [this]() {
            const QString folderName = QInputDialog::getText(
                this,
                "New Folder",
                "Folder name:");
            if (folderName.trimmed().isEmpty()) {
                return;
            }

            emit createFolderRequested(folderName.trimmed());
        });
    connect(
        m_removeFolderButton,
        &QPushButton::clicked,
        this,
        [this]() {
            QListWidgetItem *item = m_favoriteListWidget->currentItem();
            if (m_viewMode != ViewMode::FolderList || item == nullptr) {
                return;
            }

            emit folderRemoveRequested(
                item->data(Qt::UserRole).toLongLong(),
                item->toolTip().isEmpty() ? item->text() : item->toolTip());
        });
    connect(
        m_removeButton,
        &QPushButton::clicked,
        this,
        [this]() {
            QListWidgetItem *item = m_favoriteListWidget->currentItem();
            if (m_viewMode != ViewMode::FavoriteList
                || m_currentFolderId < 0
                || item == nullptr) {
                return;
            }

            emit favoriteRemoveRequested(
                m_currentFolderId,
                m_currentFolderName,
                item->data(Qt::UserRole).toString());
        });
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
            m_removeFolderButton->setEnabled(
                m_viewMode == ViewMode::FolderList && current != nullptr);
            m_removeButton->setEnabled(
                m_viewMode == ViewMode::FavoriteList && current != nullptr);
        });
}

void FavoritePage::showFoldersUnavailable()
{
    m_favoriteListWidget->clear();
    setViewMode(ViewMode::FolderList);
    m_statusLabel->setText("Loading favorite folders...");
    m_removeFolderButton->setEnabled(false);
    m_removeButton->setEnabled(false);
}

void FavoritePage::showFolders(const QList<FavoriteFolderInfo> &folders)
{
    m_favoriteListWidget->clear();
    setViewMode(ViewMode::FolderList);
    m_removeFolderButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    if (folders.isEmpty()) {
        m_statusLabel->setText("No favorite folders yet.");
        return;
    }

    m_statusLabel->clear();
    for (const FavoriteFolderInfo &folder : folders) {
        const QString displayText = QString("%1 (%2)")
                                        .arg(folder.name, QString::number(folder.problemCount));
        auto *item = new QListWidgetItem(displayText, m_favoriteListWidget);
        item->setData(Qt::UserRole, folder.id);
        item->setToolTip(folder.name);
    }
}

void FavoritePage::showRefreshUnavailable()
{
    if (m_viewMode == ViewMode::FolderList) {
        m_statusLabel->setText("Favorite folders refreshed.");
    } else {
        m_statusLabel->setText("Favorite list refreshed.");
    }
}

void FavoritePage::showFavoriteOperationFailed(const QString &message)
{
    m_statusLabel->setText(message);
}

void FavoritePage::showFavorites(qint64 folderId,
                                 const QString &folderName,
                                 const QList<ProblemPageInfo> &favorites)
{
    m_favoriteListWidget->clear();
    m_currentFolderId = folderId;
    m_currentFolderName = folderName;
    setViewMode(ViewMode::FavoriteList, folderName);
    m_removeFolderButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    if (favorites.isEmpty()) {
        m_statusLabel->setText("This folder has no favorite problems.");
        return;
    }

    m_statusLabel->clear();
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
    if (m_sectionLabel == nullptr
        || m_newFolderButton == nullptr
        || m_removeFolderButton == nullptr
        || m_removeButton == nullptr) {
        return;
    }

    if (mode == ViewMode::FolderList) {
        m_sectionLabel->setText("Favorite Folders");
        m_newFolderButton->setVisible(true);
        m_removeFolderButton->setVisible(true);
        m_removeButton->setVisible(false);
    } else {
        m_sectionLabel->setText(
            folderName.isEmpty() ? QString("Favorite Problems")
                                 : QString("Favorite Problems · %1").arg(folderName));
        m_newFolderButton->setVisible(false);
        m_removeFolderButton->setVisible(false);
        m_removeButton->setVisible(true);
    }
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
        "#favoriteTitleLabel, #favoriteSectionLabel {"
        "  color: #d9e1e8;"
        "}"
        "#favoriteActionButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#favoriteActionButton:hover { background: #26313c; }"
        "#favoriteStatusLabel { color: #f0b48a; }"
        "#favoriteListWidget { color: #e8edf2; }"
        "#favoriteListWidget::item:selected {"
        "  background: #234257;"
        "  color: #eff8ff;"
        "}"
        "#favoriteListWidget::item:hover {"
        "  background: #26313c;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
