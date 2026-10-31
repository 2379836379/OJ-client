#include "ui/pages/urlspage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString urlsStyleSheet(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QStringLiteral(
               "#settingsSaveButton {"
               "  padding: 10px 14px;"
               "  border: 1px solid %1;"
               "  border-radius: 10px;"
               "  background: %2;"
               "  color: %3;"
               "}"
               "#settingsSaveButton:hover { background: %4; }"
               "#settingsTitleLabel { font-size: 26px; font-weight: 600; color: %3; }"
               "#settingsFieldLabel, #settingsStatusLabel { font-size: 15px; color: %3; }"
               "#settingsFieldLabel { font-weight: 600; }"
               "#settingsLineEdit {"
               "  background: %2;"
               "  border: 1px solid %1;"
               "  border-radius: 12px;"
               "  color: %3;"
               "  selection-background-color: %5;"
               "  padding: 8px 10px;"
               "}")
        .arg(colors.border, colors.page, colors.text, colors.hover, colors.selected);
}
}

UrlsPage::UrlsPage(QWidget *parent)
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

    setStyleSheet(urlsStyleSheet(false));

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveRequested(m_serverUrlEdit->text().trimmed());
    });
}

void UrlsPage::setServerUrl(const QString &serverUrl)
{
    m_serverUrlEdit->setText(serverUrl);
    m_statusLabel->clear();
}

void UrlsPage::showSaveSucceeded()
{
    m_statusLabel->setText("Saved. Changes applied.");
}

void UrlsPage::showSaveFailed(const QString &message)
{
    m_statusLabel->setText(message);
}

void UrlsPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    setStyleSheet(urlsStyleSheet(dark));
}
