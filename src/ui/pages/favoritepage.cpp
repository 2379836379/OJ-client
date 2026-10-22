#include "ui/pages/favoritepage.h"
#include "ui/lightmodeiconhelper.h"

#include <QFrame>
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
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("favoriteTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("Favorites", topFrame);
    titleLabel->setObjectName("favoriteTitleLabel");
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("favoriteTopActionButton");
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("favoriteTopActionButton");
    auto *refreshButton = new QPushButton("Refresh", topFrame);
    refreshButton->setObjectName("favoriteTopActionButton");
    auto *exportButton = new QPushButton("Export", topFrame);
    exportButton->setObjectName("favoriteTopActionButton");
    exportButton->setToolTip("Export all favorite folders to a JSON file");
    auto *importButton = new QPushButton("Import", topFrame);
    importButton->setObjectName("favoriteTopActionButton");
    importButton->setToolTip("Import favorite folders from a JSON file (merge)");

    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(homeButton);
    topLayout->addWidget(themeButton);
    topLayout->addWidget(refreshButton);
    topLayout->addWidget(exportButton);
    topLayout->addWidget(importButton);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("favoriteLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("favoriteToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);
    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("favoriteToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);
    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("favoriteToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("favoriteContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(14);

    m_sectionLabel = new QLabel("Favorite Folders", contentFrame);
    m_sectionLabel->setObjectName("favoriteSectionLabel");

    m_statusLabel = new QLabel(contentFrame);
    m_statusLabel->setObjectName("favoriteStatusLabel");
    m_statusLabel->setWordWrap(true);

    m_favoriteListWidget = new QListWidget(contentFrame);
    m_favoriteListWidget->setObjectName("favoriteListWidget");
    m_favoriteListWidget->setMinimumHeight(420);

    auto *folderActionsLayout = new QHBoxLayout();
    folderActionsLayout->setSpacing(10);
    m_newFolderButton = new QPushButton("New Folder", contentFrame);
    m_newFolderButton->setObjectName("favoriteActionButton");
    m_removeFolderButton = new QPushButton("Delete Folder", contentFrame);
    m_removeFolderButton->setObjectName("favoriteActionButton");
    m_removeFolderButton->setEnabled(false);
    folderActionsLayout->addWidget(m_newFolderButton);
    folderActionsLayout->addWidget(m_removeFolderButton);
    folderActionsLayout->addStretch();

    m_removeButton = new QPushButton("Remove Selected", contentFrame);
    m_removeButton->setObjectName("favoriteActionButton");
    m_removeButton->setVisible(false);
    m_removeButton->setEnabled(false);

    contentLayout->addWidget(m_sectionLabel);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(m_favoriteListWidget, 1);
    contentLayout->addLayout(folderActionsLayout);
    contentLayout->addWidget(m_removeButton, 0, Qt::AlignLeft);

    bottomLayout->addWidget(m_toolsFrame, 1);
    bottomLayout->addWidget(contentFrame, 4);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    refreshButton->setToolTip("Refresh");
    m_backToolButton->setToolTip("Back");
    m_collapsedBackButton->setToolTip("Back");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(refreshButton, "refresh.svg");
    LightModeIconHelper::applyIcon(m_collapsedBackButton, "back.svg");

    setStyleSheet(
        "FavoritePage { background: #f3f1eb; }"
        "#favoriteTopFrame, #favoriteLeftFrame, #favoriteContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#favoriteToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#favoriteToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#favoriteToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#favoriteToolButton:hover, #favoriteToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
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
        "#favoriteTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#favoriteTopActionButton:hover {"
        "  background: transparent;"
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
        "#favoriteToolIconButton {"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 36px;"
        "  max-height: 36px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  font-weight: 600;"
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

    setToolsExpanded(true);

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
    connect(m_backToolButton, &QPushButton::clicked, this, handleBack);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, handleBack);
    connect(homeButton, &QPushButton::clicked, this, &FavoritePage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(refreshButton, &QPushButton::clicked, this, &FavoritePage::refreshRequested);
    connect(exportButton, &QPushButton::clicked, this, &FavoritePage::exportRequested);
    connect(importButton, &QPushButton::clicked, this, &FavoritePage::importRequested);
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

void FavoritePage::setToolsExpanded(bool expanded)
{
    m_toolsExpanded = expanded;
    if (m_toolsFrame != nullptr) {
        m_toolsFrame->setMinimumWidth(expanded ? 0 : 84);
        m_toolsFrame->setMaximumWidth(expanded ? QWIDGETSIZE_MAX : 84);
    }
    if (m_toolsPanel != nullptr) {
        m_toolsPanel->setVisible(expanded);
    }
    if (m_collapsedToolsPanel != nullptr) {
        m_collapsedToolsPanel->setVisible(!expanded);
    }
    if (m_collapsedBackButton != nullptr) {
        m_collapsedBackButton->setVisible(!expanded);
    }
    if (m_toolsToggleButton != nullptr) {
        m_toolsToggleButton->setToolTip(expanded ? "Collapse Tools" : "Expand Tools");
        LightModeIconHelper::applyToolsToggleIcon(m_toolsToggleButton, expanded);
    }
}

void FavoritePage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "FavoritePage { background: #000000; }"
        "#favoriteTopFrame, #favoriteLeftFrame, #favoriteContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#favoriteTitleLabel, #favoriteSectionLabel, #favoriteToolsToggleButton, #favoriteToolButton, #favoriteToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#favoriteTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#favoriteActionButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#favoriteTopActionButton:hover, #favoriteActionButton:hover, #favoriteToolButton:hover, #favoriteToolIconButton:hover {"
        "  background: #26313c;"
        "}"
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
