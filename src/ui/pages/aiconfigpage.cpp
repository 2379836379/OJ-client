#include "ui/pages/aiconfigpage.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

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

    auto *editorLabel = new QLabel("config.toml", m_shell);
    editorLabel->setObjectName("aiConfigFieldLabel");
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
    contentLayout->addWidget(editorLabel);
    contentLayout->addWidget(m_configEdit, 1);
    contentLayout->addWidget(m_statusLabel);
    contentLayout->addWidget(saveButton, 0, Qt::AlignLeft);

    m_shell->setDarkMode(false);

    setStyleSheet(
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
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "#aiConfigTitleLabel, #aiConfigFieldLabel, #aiConfigPathLabel, #aiConfigStatusLabel {"
        "  color: #d9e1e8;"
        "}"
        "#aiConfigSaveButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#aiConfigSaveButton:hover { background: #26313c; }"
        "#aiConfigPromptEdit {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
}
