#include "ui/pages/contestpage.h"

#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

namespace
{
QString problemListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 0px; border-radius: 4px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: transparent; color: %1; }"
               "QListWidget::item:hover { background: %2; }")
        .arg(colors.text, colors.hover);
}
}

ContestPage::ContestPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    m_shell->contentLayout()->setContentsMargins(20,18,20,18);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);

    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    m_titleLabel = new QLabel("Contest Set", m_shell);
    m_titleLabel->setObjectName("contestTitleLabel");
    m_problemListWidget = new QListWidget(m_shell);
    m_problemListWidget->setMinimumHeight(420);

    m_shell->contentLayout()->addWidget(m_titleLabel);
    m_shell->contentLayout()->addWidget(m_problemListWidget, 1);

    connect(m_problemListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit problemSelected(item->text(), item->data(Qt::UserRole).toString());
    });
    setDarkMode(false);
}

void ContestPage::openContest(const QString &title, const QString &url)
{
    m_contestTitle = title;
    m_contestUrl = url;
    m_titleLabel->setText(title.isEmpty() ? QStringLiteral("Contest Set") : title);
    m_problemListWidget->clear();
}

void ContestPage::showLoadingProblems()
{
}

void ContestPage::showContestLoadFailed(const QString &message)
{
    m_problemListWidget->clear();
    auto *item = new QListWidgetItem(message, m_problemListWidget);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
}

void ContestPage::showProblems(const ContestPageInfo &contestPageInfo)
{
    m_problemListWidget->clear();
    m_titleLabel->setText(m_contestTitle.isEmpty() ? QStringLiteral("Contest Set") : m_contestTitle);
    for (const ContestProblemInfo &problem : contestPageInfo.problems) {
        const QString text = QString("%1 %2").arg(problem.problemId, problem.title);
        auto *item = new QListWidgetItem(text, m_problemListWidget);
        item->setData(Qt::UserRole, problem.problemUrl);
    }
}

void ContestPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    const UiTheme::Palette colors = UiTheme::palette(dark);
    m_shell->setStyleSheet(
        UiTheme::pageShellStyleSheet(dark)
        + QString("#contestTitleLabel { color: %1; font-size: 24px; font-weight: 600; }"
                  "#contestSectionLabel { color: %1; font-size: 16px; font-weight: 600; }")
              .arg(colors.text));
    m_problemListWidget->setStyleSheet(problemListStyle(dark));
}
