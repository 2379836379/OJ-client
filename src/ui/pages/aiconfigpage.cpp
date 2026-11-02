#include "ui/pages/aiconfigpage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString aiConfigStyleSheet(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QStringLiteral(
               "#aiConfigSaveButton {"
               "  padding: 10px 14px;"
               "  border: 1px solid %1;"
               "  border-radius: 10px;"
               "  background: %2;"
               "  color: %3;"
               "}"
               "#aiConfigSaveButton:hover { background: %5; }"
               "#aiConfigTitleLabel { font-size: 26px; font-weight: 600; color: %3; }"
               "#aiConfigFieldLabel { font-size: 15px; font-weight: 600; color: %3; }"
               "#aiConfigPathLabel, #aiConfigStatusLabel { color: %3; }"
               "#aiConfigPromptEdit {"
               "  background: %2;"
               "  border: 1px solid %1;"
               "  border-radius: 12px;"
               "  color: %3;"
               "  padding: 8px 10px;"
               "  selection-color: %3;"
               "  selection-background-color: %4;"
               "}")
        .arg(colors.border, colors.page, colors.text, colors.selected, colors.hover);
}
}

AiConfigPage::AiConfigPage(QWidget *parent)
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
    auto *titleLabel = new QLabel("AI Config", m_shell);
    titleLabel->setObjectName("aiConfigTitleLabel");
    contentLayout->addWidget(titleLabel);

    auto *pathTitle = new QLabel("Config File", m_shell);
    pathTitle->setObjectName("aiConfigFieldLabel");
    m_pathLabel = new QLabel("--", m_shell);
    m_pathLabel->setObjectName("aiConfigPathLabel");
    m_pathLabel->setWordWrap(true);

    m_configEdit = new QPlainTextEdit(m_shell);
    m_configEdit->setObjectName("aiConfigPromptEdit");
    m_configEdit->setMinimumHeight(360);

    m_statusLabel = new QLabel(m_shell);
    m_statusLabel->setObjectName("aiConfigStatusLabel");
    m_statusLabel->setWordWrap(true);

    auto *saveButton = new QPushButton("Save", m_shell);
    saveButton->setObjectName("aiConfigSaveButton");

    contentLayout->addWidget(pathTitle);
    contentLayout->addWidget(m_pathLabel);
    contentLayout->addWidget(m_configEdit, 1);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(saveButton, 0, Qt::AlignLeft);

    setDarkMode(false);

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

void AiConfigPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    setStyleSheet(aiConfigStyleSheet(dark));
}
