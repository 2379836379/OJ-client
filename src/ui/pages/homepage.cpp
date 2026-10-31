#include "ui/pages/homepage.h"

#include "cache/ContestPageCache.h"
#include "ui/pageshell.h"
#include "ui/uitheme.h"

#include <QFrame>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
constexpr int ReminderTitleRole = Qt::UserRole + 1;
constexpr int ReminderDeadlineRole = Qt::UserRole + 2;

class ReminderItemDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.text.clear();
        const QWidget *widget = itemOption.widget;
        const QStyle *style = widget != nullptr ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, widget);

        QString title = index.data(ReminderTitleRole).toString();
        if (title.isEmpty()) {
            title = index.data(Qt::DisplayRole).toString();
        }
        const QString deadline = index.data(ReminderDeadlineRole).toString();
        const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText,
                                                     &itemOption,
                                                     widget);
        painter->save();
        painter->setFont(itemOption.font);
        painter->setPen(itemOption.palette.color(QPalette::Text));
        if (deadline.isEmpty()) {
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, title);
        } else {
            const int deadlineWidth = qMin(textRect.width() / 2, 220);
            const QRect titleRect = textRect.adjusted(0, 0, -deadlineWidth - 12, 0);
            const QRect deadlineRect(textRect.right() - deadlineWidth + 1,
                                     textRect.top(),
                                     deadlineWidth,
                                     textRect.height());
            const QFontMetrics metrics(itemOption.font);
            painter->drawText(titleRect, Qt::AlignVCenter | Qt::AlignLeft,
                              metrics.elidedText(title, Qt::ElideRight, titleRect.width()));
            painter->drawText(deadlineRect, Qt::AlignVCenter | Qt::AlignRight,
                              deadline);
        }
        painter->restore();
    }
};

QString homeListStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QString(
               "QListWidget { background: transparent; border: none; outline: none; color: %1; }"
               "QListWidget::item { padding: 12px 0px; border-radius: 4px; margin: 2px 0px; }"
               "QListWidget::item:selected { background: transparent; color: %1;}"
               "QListWidget::item:hover { background: %2; }")
        .arg(colors.text, colors.hover);
}

QString formatReminderContestTitle(const DeadlineReminder &reminder)
{
    return reminder.contestTitle;
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
    m_shell = new PageShell(this);
    m_shell->contentLayout()->setContentsMargins(0,0,0,0);
    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(m_shell);

    m_homeToolButton = m_shell->addTool("Home", "Home", [] {},
                                        "homepage.svg", &m_collapsedHomeToolButton);
    m_homeToolButton->setProperty("active", true);
    m_collapsedHomeToolButton->setProperty("active", true);
    m_shell->addThemeTool([this]() { emit themeToggleRequested(!m_darkMode); });
    m_shell->addTool("Refresh", "Refresh", [this]() {
        emit refreshRequested();
    }, "refresh.svg");
    m_shell->addTool("Open Favorites","Favorites", [this]() {
        emit favoritesRequested();
    }, "favorite.svg");
    m_shell->addTool("Task Board","Task Board", [this]() {
        emit tasksRequested();
    }, "taskboard.svg");
    m_shell->addTool("Stats","Stats", [this]() {
        emit statsRequested();
    }, "stats.svg");
    m_shell->addTool("Review", "Review", [this]() {
        emit reviewRequested();
    }, "review.svg");
    m_shell->addTool("Set", "Set", [this]() {
        emit storageRequested();
    }, "set.svg");
    m_shell->addTool("URLs", "Server URL", [this]() {
        emit settingsRequested();
    }, "urls.svg");
    m_shell->addTool("AI Config", "AI Config", [this]() {
        emit aiConfigRequested();
    }, "aiconfig.svg");
    m_shell->addTool("Log Out", "Log Out", [this]() {
        emit logoutRequested();
    }, "logout.svg");
    m_shell->toolsLayout()->addStretch();
    m_shell->collapsedToolsLayout()->addStretch();

    auto *columnsLayout = new QHBoxLayout();
    columnsLayout->setContentsMargins(0, 0, 0, 0);
    columnsLayout->setSpacing(UiTheme::PageSpacing);

    auto *classesFrame = new QFrame(m_shell);
    classesFrame->setObjectName("homeCourseFrame");
    auto *classesColumn = new QVBoxLayout(classesFrame);
    classesColumn->setContentsMargins(20,18,20,18);
    classesColumn->setSpacing(14);
    auto *classesTitle = new QLabel("Course List", classesFrame);
    classesTitle->setObjectName("homeSectionLabel");
    m_classListWidget = new QListWidget(classesFrame);
    m_classListWidget->setObjectName("homeClassList");
    m_classListWidget->setMinimumHeight(420);
    classesColumn->addWidget(classesTitle);
    classesColumn->addWidget(m_classListWidget, 1);

    auto *remindersFrame = new QFrame(m_shell);
    remindersFrame->setObjectName("homeDueFrame");
    auto *remindersColumn = new QVBoxLayout(remindersFrame);
    remindersColumn->setContentsMargins(20,18,20,18);
    remindersColumn->setSpacing(14);
    auto *remindersTitle = new QLabel("Due Soon", remindersFrame);
    remindersTitle->setObjectName("homeSectionLabel");
    m_reminderListWidget = new QListWidget(remindersFrame);
    m_reminderListWidget->setObjectName("homeReminderList");
    m_reminderListWidget->setItemDelegate(new ReminderItemDelegate(m_reminderListWidget));
    remindersColumn->addWidget(remindersTitle);
    remindersColumn->addWidget(m_reminderListWidget, 1);

    columnsLayout->addWidget(classesFrame, 1);
    columnsLayout->addWidget(remindersFrame, 1);
    m_shell->contentLayout()->addLayout(columnsLayout, 1);

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
        + QString("#pageShellContent { background: transparent; border: none; }"
                  "#homeCourseFrame, #homeDueFrame { background: %1; border: 1px solid %2; border-radius: %3px; }"
                  "#homeSectionLabel { color: %4; font-size: 16px; font-weight: 600; }")
              .arg(colors.page,
                   colors.border,
                   QString::number(UiTheme::PanelRadius),
                   colors.text));
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
        auto *item = new QListWidgetItem(formatReminderContestTitle(reminder),
                                         m_reminderListWidget);
        item->setData(Qt::UserRole, reminder.contestUrl);
        item->setData(ReminderTitleRole, reminder.contestTitle);
        item->setData(ReminderDeadlineRole, reminder.deadlineText);
        item->setToolTip(
            QString("%1\n%2").arg(reminder.courseName, reminder.deadlineText));
    }
}
