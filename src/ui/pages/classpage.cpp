#include "ui/pages/classpage.h"
#include "ui/lightmodeiconhelper.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

ClassPage::ClassPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("classTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    m_titleLabel = new QLabel("Course", topFrame);
    m_titleLabel->setObjectName("classTitleLabel");

    topLayout->addWidget(m_titleLabel, 1);
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("classTopActionButton");
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("classTopActionButton");
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);
    auto *refreshButton = new QPushButton("Refresh", topFrame);
    refreshButton->setObjectName("classTopActionButton");
    topLayout->addWidget(refreshButton, 0, Qt::AlignRight);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("classLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("classToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);

    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("classToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    m_collapsedToolsPanel->setObjectName("classCollapsedToolsPanel");
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);

    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("classToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("classContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(14);

    auto *contestLabel = new QLabel("Contest List", contentFrame);
    contestLabel->setObjectName("classSectionLabel");

    m_contestListWidget = new QListWidget(contentFrame);
    m_contestListWidget->setObjectName("classContestList");
    m_contestListWidget->setMinimumHeight(420);

    contentLayout->addWidget(contestLabel);
    contentLayout->addWidget(m_contestListWidget, 1);

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
        "ClassPage { background: #f3f1eb; }"
        "#classTopFrame, #classLeftFrame, #classContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#classTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#classSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#classToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#classToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#classTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #243029;"
        "}"
        "#classTopActionButton:hover {"
        "  background: transparent;"
        "}"
        "#classToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#classToolButton:hover, #classToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#classToolIconButton {"
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
        "#classContestList {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0px;"
        "  padding: 0px;"
        "  outline: none;"
        "}"
        "#classContestList::item {"
        "  padding: 12px 4px;"
        "  border-radius: 8px;"
        "  margin: 2px 0px;"
        "}"
        "#classContestList::item:selected {"
        "  background: #dcefea;"
        "  color: #12343b;"
        "}"
        "#classContestList::item:hover {"
        "  background: #eef4ef;"
        "}"
    );

    setToolsExpanded(true);

    connect(homeButton, &QPushButton::clicked, this, &ClassPage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(refreshButton, &QPushButton::clicked, this, &ClassPage::refreshRequested);
    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &ClassPage::refreshRequested);
    connect(m_backToolButton, &QPushButton::clicked, this, &ClassPage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &ClassPage::backRequested);
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(
        m_contestListWidget,
        &QListWidget::itemClicked,
        this,
        [this](QListWidgetItem *item) {
            emit contestSelected(
                item->text(),
                item->data(Qt::UserRole).toString());
        });
}

void ClassPage::setToolsExpanded(bool expanded)
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

void ClassPage::openClass(const QString &name, const QString &url)
{
    m_className = name;
    m_classUrl = url;
    m_titleLabel->setText(name.isEmpty() ? QString("Course") : name);
    m_contestListWidget->clear();
}

void ClassPage::showLoadingContests()
{
}

void ClassPage::showClassLoadFailed(const QString &message)
{
    m_contestListWidget->clear();
    auto *item = new QListWidgetItem(message, m_contestListWidget);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
}

void ClassPage::showContestSets(const GroupPageInfo &groupPageInfo)
{
    m_contestListWidget->clear();
    for (const ContestSetInfo &contestSet : groupPageInfo.contestSets) {
        QString text = contestSet.title;
        if (!contestSet.extraText.isEmpty()) {
            text += " | " + contestSet.extraText;
        }
        auto *item = new QListWidgetItem(text, m_contestListWidget);
        item->setData(Qt::UserRole, contestSet.url);
    }
}

void ClassPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "ClassPage { background: #000000; }"
        "#classTopFrame, #classLeftFrame, #classContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#classTitleLabel, #classSectionLabel, #classToolsToggleButton, #classToolButton, #classToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#classTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#classTopActionButton:hover, #classToolButton:hover, #classToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#classContestList { color: #e8edf2; }"
        "#classContestList::item:selected {"
        "  background: #234257;"
        "  color: #eff8ff;"
        "}"
        "#classContestList::item:hover {"
        "  background: #26313c;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
