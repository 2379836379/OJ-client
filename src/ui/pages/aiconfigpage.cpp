#include "ui/pages/aiconfigpage.h"
#include "ui/lightmodeiconhelper.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

AiConfigPage::AiConfigPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("aiConfigTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("AI Config", topFrame);
    titleLabel->setObjectName("aiConfigTitleLabel");
    topLayout->addWidget(titleLabel, 1);
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("aiConfigTopActionButton");
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("aiConfigTopActionButton");
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("aiConfigLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("aiConfigToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);
    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("aiConfigToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);
    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("aiConfigToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("aiConfigContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(24, 22, 24, 22);
    contentLayout->setSpacing(14);

    auto *pathTitle = new QLabel("Config File", contentFrame);
    pathTitle->setObjectName("aiConfigFieldLabel");
    m_pathLabel = new QLabel("--", contentFrame);
    m_pathLabel->setObjectName("aiConfigPathLabel");
    m_pathLabel->setWordWrap(true);

    auto *editorLabel = new QLabel("config.toml", contentFrame);
    editorLabel->setObjectName("aiConfigFieldLabel");
    m_configEdit = new QPlainTextEdit(contentFrame);
    m_configEdit->setObjectName("aiConfigPromptEdit");
    m_configEdit->setMinimumHeight(360);

    m_statusLabel = new QLabel(contentFrame);
    m_statusLabel->setObjectName("aiConfigStatusLabel");
    m_statusLabel->setWordWrap(true);

    auto *saveButton = new QPushButton("Save", contentFrame);
    saveButton->setObjectName("aiConfigSaveButton");

    contentLayout->addWidget(pathTitle);
    contentLayout->addWidget(m_pathLabel);
    contentLayout->addWidget(editorLabel);
    contentLayout->addWidget(m_configEdit, 1);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(saveButton, 0, Qt::AlignLeft);

    bottomLayout->addWidget(m_toolsFrame, 1);
    bottomLayout->addWidget(contentFrame, 4);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    m_backToolButton->setToolTip("Back");
    m_collapsedBackButton->setToolTip("Back");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(m_collapsedBackButton, "back.svg");

    setStyleSheet(
        "AiConfigPage { background: #f3f1eb; }"
        "#aiConfigTopFrame, #aiConfigLeftFrame, #aiConfigContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#aiConfigToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#aiConfigToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#aiConfigToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#aiConfigToolButton:hover, #aiConfigToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#aiConfigTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#aiConfigTopActionButton:hover {"
        "  background: transparent;"
        "}"
        "#aiConfigSaveButton {"
        "  padding: 10px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#aiConfigSaveButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#aiConfigToolIconButton {"
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
        "#aiConfigTitleLabel { font-size: 26px; font-weight: 600; color: #1f2328; }"
        "#aiConfigFieldLabel { font-size: 15px; font-weight: 600; color: #2f3a33; }"
        "#aiConfigPathLabel, #aiConfigStatusLabel { color: #526056; }"
        "#aiConfigPromptEdit {"
        "  background: #fcfbf8;"
        "  border: 1px solid #d9d4c8;"
        "  border-radius: 12px;"
        "  color: #1f2328;"
        "  padding: 8px 10px;"
        "}"
    );

    setToolsExpanded(true);

    connect(homeButton, &QPushButton::clicked, this, &AiConfigPage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_backToolButton, &QPushButton::clicked, this, &AiConfigPage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &AiConfigPage::backRequested);
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveRequested(currentConfigText());
    });
}

void AiConfigPage::setConfigText(const QString &path, const QString &content)
{
    m_pathLabel->setText(path.isEmpty() ? "config.toml" : path);
    m_configEdit->setPlainText(content);
    m_statusLabel->clear();
}

QString AiConfigPage::currentConfigText() const
{
    return m_configEdit->toPlainText();
}

void AiConfigPage::showSaveSucceeded(const QString &path)
{
    m_pathLabel->setText(path);
    m_statusLabel->setText("Saved.");
}

void AiConfigPage::showSaveFailed(const QString &message)
{
    m_statusLabel->setText(message);
}

void AiConfigPage::setToolsExpanded(bool expanded)
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

void AiConfigPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "AiConfigPage { background: #000000; }"
        "#aiConfigTopFrame, #aiConfigLeftFrame, #aiConfigContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#aiConfigTitleLabel, #aiConfigFieldLabel, #aiConfigPathLabel, #aiConfigStatusLabel, #aiConfigToolsToggleButton, #aiConfigToolButton, #aiConfigToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#aiConfigTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#aiConfigSaveButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#aiConfigTopActionButton:hover, #aiConfigSaveButton:hover, #aiConfigToolButton:hover, #aiConfigToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#aiConfigPromptEdit {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
