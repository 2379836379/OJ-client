#include "ui/pages/classpage.h"

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
constexpr int ContentPaddingX = 20;
constexpr int ContentPaddingY = 18;

QString contestListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 4px; border-radius: 8px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: %2; color: %1; }"
               "QListWidget::item:hover { background: %3; }")
        .arg(colors.text, colors.selected, colors.hover);
}
}

ClassPage::ClassPage(QWidget *parent)
    : QWidget(parent)
{
    m_shell = new PageShell(this);
    m_shell->contentLayout()->setContentsMargins(ContentPaddingX,
                                                  ContentPaddingY,
                                                  ContentPaddingX,
                                                  ContentPaddingY);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_shell);

    m_shell->addTool("Home", "Home", [this]() { emit homeRequested(); }, "homepage.svg");
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() { emit refreshRequested(); }, "refresh.svg");
    m_shell->addTool("Back", "Back", [this]() { emit backRequested(); }, "back.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    m_titleLabel = new QLabel("Course", m_shell);
    m_titleLabel->setObjectName("classTitleLabel");
    auto *contestLabel = new QLabel("Contest List", m_shell);
    contestLabel->setObjectName("classSectionLabel");
    m_contestListWidget = new QListWidget(m_shell);
    m_contestListWidget->setMinimumHeight(420);

    m_shell->contentLayout()->addWidget(m_titleLabel);
    m_shell->contentLayout()->addWidget(contestLabel);
    m_shell->contentLayout()->addWidget(m_contestListWidget, 1);

    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &ClassPage::refreshRequested);
    connect(m_contestListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit contestSelected(item->text(), item->data(Qt::UserRole).toString());
    });
    setDarkMode(false);
}

void ClassPage::openClass(const QString &name, const QString &url)
{
    m_className = name;
    m_classUrl = url;
    m_titleLabel->setText(name.isEmpty() ? QStringLiteral("Course") : name);
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
        if (contestSet.problemnumber > 0) {
            text += QString(" (%1 problems)").arg(contestSet.problemnumber);
        }
        if (!contestSet.endtime.isEmpty()) {
            text += " | " + contestSet.endtime;
        }
        auto *item = new QListWidgetItem(text, m_contestListWidget);
        item->setData(Qt::UserRole, contestSet.url);
    }
}

void ClassPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    const UiTheme::Palette colors = UiTheme::palette(dark);
    m_shell->setStyleSheet(
        UiTheme::pageShellStyleSheet(dark)
        + QString("#classTitleLabel { color: %1; font-size: 24px; font-weight: 600; }"
                  "#classSectionLabel { color: %1; font-size: 16px; font-weight: 600; }")
              .arg(colors.text));
    m_contestListWidget->setStyleSheet(contestListStyle(dark));
}
