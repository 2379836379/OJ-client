#include "ui/pages/homepage.h"

#include "cache/ContestPageCache.h"
#include "ui/lightmodeiconhelper.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QShortcut>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
QString homeListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 4px; border-radius: 8px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: %2; color: %3; }"
               "QListWidget::item:hover { background: %4; }")
        .arg(colors.text, colors.selected, colors.accentText, colors.hover);
}

QString formatReminderContestTitle(const DeadlineReminder &reminder,
                                   ContestPageCache *contestCache)
{
    QString text = reminder.contestTitle;
    ContestPageInfo contestPageInfo;
    if (contestCache != nullptr
        && contestCache->load(reminder.contestUrl, &contestPageInfo)
        && contestPageInfo.totalProblems > 0) {
        text += QString(" (%1/%2)")
                    .arg(QString::number(contestPageInfo.solvedProblems),
                         QString::number(contestPageInfo.totalProblems));
    }
    if (!reminder.deadlineText.isEmpty()) {
        text += " | " + reminder.deadlineText;
    }
    return text;
}

bool shouldHideCompletedReminder(const DeadlineReminder &reminder,
                                 ContestPageCache *contestCache)
{
    ContestPageInfo contestPageInfo;
    if (contestCache == nullptr
        || !contestCache->load(reminder.contestUrl, &contestPageInfo)) {
        return false;
    }
    return contestPageInfo.totalProblems > 0
        && contestPageInfo.solvedProblems >= contestPageInfo.totalProblems;
}
}

HomePage::HomePage(ContestPageCache *contestCache, QWidget *parent)
    : QWidget(parent)
    , m_contestCache(contestCache)
{
    m_shell = new PageShell(QStringLiteral("OJ"), this);
    m_shell->setContentFramed(false);
    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(m_shell);

    auto *homeButton = m_shell->addHeaderAction(QString(), "pageShellActionButton", "Home");
    m_themeButton = m_shell->addHeaderAction(QString(), "pageShellActionButton", "Dark Mode");
    auto *refreshButton = m_shell->addHeaderAction(QString(), "pageShellActionButton", "Refresh");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(m_themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(refreshButton, "refresh.svg");

    const auto addTool = [this](const QString &text,
                                const QString &iconText,
                                const QString &toolTip,
                                void (HomePage::*request)()) {
        auto *button = new QPushButton(text, m_shell);
        button->setObjectName("pageShellToolButton");
        m_shell->toolsLayout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, request);

        auto *iconButton = new QPushButton(iconText, m_shell);
        iconButton->setObjectName("pageShellToolIconButton");
        iconButton->setFixedSize(36, 36);
        iconButton->setToolTip(toolTip);
        m_shell->collapsedToolsLayout()->addWidget(iconButton);
        connect(iconButton, &QPushButton::clicked, this, request);
    };

    addTool("Open Favorites", "F", "Favorites", &HomePage::favoritesRequested);
    addTool("Task Board", "T", "Task Board", &HomePage::tasksRequested);
    addTool("Stats", "St", "Stats", &HomePage::statsRequested);
    addTool("Review", "R", "Review", &HomePage::reviewRequested);

    auto *storageButton = new QPushButton("Set", m_shell);
    storageButton->setObjectName("pageShellToolButton");
    storageButton->setToolTip("Set");
    m_shell->toolsLayout()->addWidget(storageButton);
    connect(storageButton, &QPushButton::clicked, this, &HomePage::storageRequested);
    auto *collapsedStorageButton = new QPushButton("S", m_shell);
    collapsedStorageButton->setObjectName("pageShellToolIconButton");
    collapsedStorageButton->setFixedSize(36, 36);
    collapsedStorageButton->setToolTip("Set");
    LightModeIconHelper::applyIcon(collapsedStorageButton, "set.svg");
    m_shell->collapsedToolsLayout()->addWidget(collapsedStorageButton);
    connect(collapsedStorageButton, &QPushButton::clicked, this, &HomePage::storageRequested);

    addTool("URLs", "U", "Server URL", &HomePage::settingsRequested);
    auto *aiButton = new QPushButton("AI Config", m_shell);
    aiButton->setObjectName("pageShellToolButton");
    aiButton->setToolTip("AI Config");
    m_shell->toolsLayout()->addWidget(aiButton);
    connect(aiButton, &QPushButton::clicked, this, &HomePage::aiConfigRequested);
    auto *collapsedAiButton = new QPushButton("A", m_shell);
    collapsedAiButton->setObjectName("pageShellToolIconButton");
    collapsedAiButton->setFixedSize(36, 36);
    collapsedAiButton->setToolTip("AI Config");
    LightModeIconHelper::applyIcon(collapsedAiButton, "ai-config.svg");
    m_shell->collapsedToolsLayout()->addWidget(collapsedAiButton);
    connect(collapsedAiButton, &QPushButton::clicked, this, &HomePage::aiConfigRequested);

    addTool("Log Out", "L", "Log Out", &HomePage::logoutRequested);
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *columnsLayout = new QHBoxLayout();
    columnsLayout->setContentsMargins(0, 0, 0, 0);
    columnsLayout->setSpacing(UiTheme::PageSpacing);

    auto *classesColumn = new QVBoxLayout();
    classesColumn->setContentsMargins(0, 0, 0, 0);
    classesColumn->setSpacing(14);
    auto *classesTitle = new QLabel("Course List", m_shell);
    classesTitle->setObjectName("homeSectionLabel");
    m_classListWidget = new QListWidget(m_shell);
    m_classListWidget->setObjectName("homeClassList");
    m_classListWidget->setMinimumHeight(420);
    classesColumn->addWidget(classesTitle);
    classesColumn->addWidget(m_classListWidget, 1);

    auto *remindersColumn = new QVBoxLayout();
    remindersColumn->setContentsMargins(0, 0, 0, 0);
    remindersColumn->setSpacing(14);
    auto *remindersTitle = new QLabel("Due Soon", m_shell);
    remindersTitle->setObjectName("homeSectionLabel");
    m_reminderListWidget = new QListWidget(m_shell);
    m_reminderListWidget->setObjectName("homeReminderList");
    remindersColumn->addWidget(remindersTitle);
    remindersColumn->addWidget(m_reminderListWidget, 1);

    columnsLayout->addLayout(classesColumn, 1);
    columnsLayout->addLayout(remindersColumn, 1);
    m_shell->contentLayout()->addLayout(columnsLayout, 1);

    connect(refreshButton, &QPushButton::clicked, this, &HomePage::refreshRequested);
    auto *refreshShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_R), this);
    refreshShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshShortcut, &QShortcut::activated, this, &HomePage::refreshRequested);
    connect(m_themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_classListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        emit classSelected(item->text(), item->data(Qt::UserRole).toString());
    });
    connect(m_reminderListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const QString contestUrl = item->data(Qt::UserRole).toString();
        const QString contestTitle = item->data(Qt::UserRole + 1).toString();
        const QUrl parsedUrl(contestUrl);
        if (!contestUrl.isEmpty() && parsedUrl.isValid()
            && !parsedUrl.scheme().isEmpty()) {
            emit reminderSelected(
                contestTitle.isEmpty() ? item->text() : contestTitle,
                contestUrl);
        }
    });
    setDarkMode(false);
}

void HomePage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    m_shell->setDarkMode(dark);
    const UiTheme::Palette colors = UiTheme::palette(dark);
    m_shell->setStyleSheet(
        UiTheme::pageShellStyleSheet(dark)
        + QString("#homeSectionLabel { color: %1; font-size: 16px; font-weight: 600; }")
              .arg(colors.text));
    m_classListWidget->setStyleSheet(homeListStyle(dark));
    m_reminderListWidget->setStyleSheet(homeListStyle(dark));
}

void HomePage::showLoggingIn()
{
    showReminderLoading();
}

void HomePage::showLoginFailed(const QString &message)
{
    Q_UNUSED(message);
}

void HomePage::showOpeningHome()
{
    showReminderLoading();
}

void HomePage::showLoadingClasses()
{
    showReminderLoading();
}

void HomePage::showHomeLoadFailed(const QString &message)
{
    Q_UNUSED(message);
}

void HomePage::showClasses(const QList<JoinedClassInfo> &classes)
{
    m_classListWidget->clear();
    for (const JoinedClassInfo &joinedClass : classes) {
        const QString displayText = joinedClass.name.isEmpty()
            ? QStringLiteral("Unnamed class")
            : joinedClass.name;
        auto *item = new QListWidgetItem(displayText, m_classListWidget);
        item->setData(Qt::UserRole, joinedClass.url);
    }
}

void HomePage::showReminderLoading()
{
    m_reminderListWidget->clear();
    new QListWidgetItem("Checking course deadlines...", m_reminderListWidget);
}

void HomePage::showReminders(const QList<DeadlineReminder> &reminders)
{
    m_reminderListWidget->clear();
    QList<DeadlineReminder> visibleReminders;
    for (const DeadlineReminder &reminder : reminders) {
        if (!shouldHideCompletedReminder(reminder, m_contestCache)) {
            visibleReminders.append(reminder);
        }
    }

    std::sort(
        visibleReminders.begin(),
        visibleReminders.end(),
        [](const DeadlineReminder &lhs, const DeadlineReminder &rhs) {
            return lhs.deadline < rhs.deadline;
        });

    if (visibleReminders.isEmpty()) {
        new QListWidgetItem("No contest deadlines within one week.", m_reminderListWidget);
        return;
    }

    for (const DeadlineReminder &reminder : visibleReminders) {
        auto *item = new QListWidgetItem(
            formatReminderContestTitle(reminder, m_contestCache),
            m_reminderListWidget);
        item->setData(Qt::UserRole, reminder.contestUrl);
        item->setData(Qt::UserRole + 1, reminder.contestTitle);
        item->setToolTip(
            QString("%1\n%2").arg(reminder.courseName, reminder.deadlineText));
    }
}
