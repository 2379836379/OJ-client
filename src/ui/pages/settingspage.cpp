#include "ui/pages/settingspage.h"
#include "ui/lightmodeiconhelper.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(18);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("settingsTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    auto *titleLabel = new QLabel("Server URLs", topFrame);
    titleLabel->setObjectName("settingsTitleLabel");
    topLayout->addWidget(titleLabel, 1);
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("settingsTopActionButton");
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("settingsTopActionButton");
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(18);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("settingsLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("settingsToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);
    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("settingsToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);
    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("settingsToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();

    auto *contentFrame = new QFrame(this);
    contentFrame->setObjectName("settingsContentFrame");
    auto *contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(24, 22, 24, 22);
    contentLayout->setSpacing(14);

    auto *openJudgeLabel = new QLabel("OpenJudge base URL", contentFrame);
    openJudgeLabel->setObjectName("settingsFieldLabel");
    m_openJudgeEdit = new QLineEdit(contentFrame);
    m_openJudgeEdit->setObjectName("settingsLineEdit");
    m_openJudgeEdit->setPlaceholderText("http://openjudge.cn");

    auto *judgerLabel = new QLabel("Judger base URL", contentFrame);
    judgerLabel->setObjectName("settingsFieldLabel");
    m_judgerEdit = new QLineEdit(contentFrame);
    m_judgerEdit->setObjectName("settingsLineEdit");
    m_judgerEdit->setPlaceholderText("http://host:18080");

    auto *emailVerifyLabel = new QLabel("Email verify URL", contentFrame);
    emailVerifyLabel->setObjectName("settingsFieldLabel");
    m_emailVerifyEdit = new QLineEdit(contentFrame);
    m_emailVerifyEdit->setObjectName("settingsLineEdit");
    m_emailVerifyEdit->setPlaceholderText("http://host:8080");

    m_statusLabel = new QLabel(contentFrame);
    m_statusLabel->setObjectName("settingsStatusLabel");
    m_statusLabel->setWordWrap(true);

    auto *saveButton = new QPushButton("Save", contentFrame);
    saveButton->setObjectName("settingsSaveButton");

    contentLayout->addWidget(openJudgeLabel);
    contentLayout->addWidget(m_openJudgeEdit);
    contentLayout->addWidget(judgerLabel);
    contentLayout->addWidget(m_judgerEdit);
    contentLayout->addWidget(emailVerifyLabel);
    contentLayout->addWidget(m_emailVerifyEdit);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addStretch();
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
        "SettingsPage { background: #f3f1eb; }"
        "#settingsTopFrame, #settingsLeftFrame, #settingsContentFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#settingsToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#settingsToolsToggleButton:hover { color: #12343b; }"
        "#settingsToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#settingsToolButton:hover, #settingsToolIconButton:hover { background: #eef4ef; }"
        "#settingsTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "}"
        "#settingsTopActionButton:hover { background: transparent; }"
        "#settingsSaveButton {"
        "  padding: 10px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#settingsSaveButton:hover { background: #eef4ef; }"
        "#settingsToolIconButton {"
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
        "#settingsTitleLabel { font-size: 26px; font-weight: 600; color: #1f2328; }"
        "#settingsFieldLabel { font-size: 15px; font-weight: 600; color: #2f3a33; }"
        "#settingsStatusLabel { color: #526056; }"
        "#settingsLineEdit {"
        "  background: #fcfbf8;"
        "  border: 1px solid #d9d4c8;"
        "  border-radius: 12px;"
        "  color: #1f2328;"
        "  padding: 8px 10px;"
        "}");

    setToolsExpanded(true);

    connect(homeButton, &QPushButton::clicked, this, &SettingsPage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_backToolButton, &QPushButton::clicked, this, &SettingsPage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &SettingsPage::backRequested);
    connect(m_toolsToggleButton, &QPushButton::clicked, this, [this]() {
        setToolsExpanded(!m_toolsExpanded);
    });
    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveRequested(m_openJudgeEdit->text().trimmed(),
                           m_judgerEdit->text().trimmed(),
                           m_emailVerifyEdit->text().trimmed());
    });
}

void SettingsPage::setUrls(const QString &openJudgeUrl,
                           const QString &judgerUrl,
                           const QString &emailVerifyUrl)
{
    m_openJudgeEdit->setText(openJudgeUrl);
    m_judgerEdit->setText(judgerUrl);
    m_emailVerifyEdit->setText(emailVerifyUrl);
    m_statusLabel->clear();
}

void SettingsPage::showSaveSucceeded()
{
    m_statusLabel->setText("Saved. Changes applied.");
}

void SettingsPage::showSaveFailed(const QString &message)
{
    m_statusLabel->setText(message);
}

void SettingsPage::setToolsExpanded(bool expanded)
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

void SettingsPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "SettingsPage { background: #000000; }"
        "#settingsTopFrame, #settingsLeftFrame, #settingsContentFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#settingsTitleLabel, #settingsFieldLabel, #settingsStatusLabel, #settingsToolsToggleButton, #settingsToolButton, #settingsToolIconButton {"
        "  color: #d9e1e8;"
        "}"
        "#settingsTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#settingsSaveButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#settingsTopActionButton:hover, #settingsSaveButton:hover, #settingsToolButton:hover, #settingsToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#settingsLineEdit {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
