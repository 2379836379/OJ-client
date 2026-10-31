#include "ui/pages/settingspage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);
    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *contentLayout = m_shell->contentLayout();
    contentLayout->setContentsMargins(24, 22, 24, 22);
    contentLayout->setSpacing(14);
    auto *titleLabel = new QLabel("Server URL", m_shell);
    titleLabel->setObjectName("settingsTitleLabel");
    contentLayout->addWidget(titleLabel);

    auto *serverLabel = new QLabel("Application server URL", m_shell);
    serverLabel->setObjectName("settingsFieldLabel");
    m_serverUrlEdit = new QLineEdit(m_shell);
    m_serverUrlEdit->setObjectName("settingsLineEdit");
    m_serverUrlEdit->setPlaceholderText("http://host:18080");

    m_statusLabel = new QLabel(m_shell);
    m_statusLabel->setObjectName("settingsStatusLabel");
    m_statusLabel->setWordWrap(true);

    auto *saveButton = new QPushButton("Save", m_shell);
    saveButton->setObjectName("settingsSaveButton");

    contentLayout->addWidget(serverLabel);
    contentLayout->addWidget(m_serverUrlEdit);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addStretch();
    contentLayout->addWidget(saveButton, 0, Qt::AlignLeft);

    m_shell->setStyleSheet(UiTheme::pageShellStyleSheet(false));

    setStyleSheet(
        "#settingsSaveButton {"
        "  padding: 10px 14px;"
        "  border: 1px solid #d7d2c7;"
        "  border-radius: 10px;"
        "  background: #f6f3ec;"
        "  color: #2f3a33;"
        "}"
        "#settingsSaveButton:hover { background: #eef4ef; }"
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

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveRequested(m_serverUrlEdit->text().trimmed());
    });
}

void SettingsPage::setServerUrl(const QString &serverUrl)
{
    m_serverUrlEdit->setText(serverUrl);
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

void SettingsPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "#settingsTitleLabel, #settingsFieldLabel, #settingsStatusLabel {"
        "  color: #d9e1e8;"
        "}"
        "#settingsSaveButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#settingsSaveButton:hover { background: #26313c; }"
        "#settingsLineEdit {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
