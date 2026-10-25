#include "reminderservice.h"

#include "browse/Group.h"

#include <algorithm>
#include <QRegularExpression>

namespace
{
bool isWithinOneWeek(const QDateTime &now, const QDateTime &deadline)
{
    const qint64 seconds = now.secsTo(deadline);
    return seconds >= 0 && seconds < 7 * 24 * 60 * 60;
}

bool sameReminders(const QList<DeadlineReminder> &lhs,
                   const QList<DeadlineReminder> &rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (int i = 0; i < lhs.size(); ++i) {
        const DeadlineReminder &left = lhs.at(i);
        const DeadlineReminder &right = rhs.at(i);
        if (left.courseName != right.courseName
            || left.contestTitle != right.contestTitle
            || left.contestUrl != right.contestUrl
            || left.deadlineText != right.deadlineText
            || left.deadline != right.deadline) {
            return false;
        }
    }

    return true;
}
}

ReminderService::ReminderService(Browse::Group *group,
                                 QObject *parent)
    : QObject(parent)
    , m_group(group)
{
    connect(
        m_group,
        &Browse::Group::loaded,
        this,
        [this](const QUrl &groupPageUrl, const GroupPageInfo &groupPageInfo) {
            m_currentGroupPageUrl = groupPageUrl;
            updateGroupReminders(
                m_currentGroup.url,
                collectReminders(m_currentGroup,
                                 m_currentGroupPageUrl,
                                 groupPageInfo));
            processNextGroup();
        });

    auto handleFailure = [this](const QString &message) {
            if (!message.isEmpty()) {
                emit failed(message);
            }
            processNextGroup();
        };
    connect(m_group, &Browse::Group::failed, this, handleFailure);
}

void ReminderService::refreshReminders(const QList<JoinedClassInfo> &groups)
{
    m_pendingGroups.clear();
    m_remindersByGroup.clear();
    for (const JoinedClassInfo &group : groups) {
        m_pendingGroups.enqueue(group);
    }

    m_loading = true;
    emit loadingChanged(true);
    processNextGroup();
}

void ReminderService::processNextGroup()
{
    if (m_pendingGroups.isEmpty()) {
        finalizeRefresh();
        return;
    }

    m_currentGroup = m_pendingGroups.dequeue();
    m_currentGroupPageUrl = {};
    m_group->open(QUrl(m_currentGroup.url));
}

QList<DeadlineReminder> ReminderService::collectReminders(
    const JoinedClassInfo &group,
    const QUrl &groupPageUrl,
    const GroupPageInfo &groupPageInfo) const
{
    QList<DeadlineReminder> reminders;
    const QDateTime now = QDateTime::currentDateTime();
    for (const ContestSetInfo &contestSet : groupPageInfo.contestSets) {
        const QDateTime deadline = parseDeadline(contestSet.endtime);
        if (!deadline.isValid() || !isWithinOneWeek(now, deadline)) {
            continue;
        }

        DeadlineReminder reminder;
        reminder.courseName = group.name;
        reminder.contestTitle = contestSet.title;
        reminder.contestUrl =
            groupPageUrl.resolved(QUrl(contestSet.url)).toString();
        reminder.deadlineText = contestSet.endtime;
        reminder.deadline = deadline;
        reminders << reminder;
    }
    return reminders;
}

void ReminderService::updateGroupReminders(const QString &groupUrl,
                                           const QList<DeadlineReminder> &reminders)
{
    const auto existingIt = m_remindersByGroup.constFind(groupUrl);
    if (existingIt != m_remindersByGroup.cend()
        && sameReminders(existingIt.value(), reminders)) {
        return;
    }

    m_remindersByGroup.insert(groupUrl, reminders);
    emitCombinedReminders();
}

void ReminderService::emitCombinedReminders()
{
    QList<DeadlineReminder> combined;
    for (auto it = m_remindersByGroup.cbegin(); it != m_remindersByGroup.cend(); ++it) {
        combined += it.value();
    }

    std::sort(
        combined.begin(),
        combined.end(),
        [](const DeadlineReminder &lhs, const DeadlineReminder &rhs) {
            return lhs.deadline < rhs.deadline;
        });

    QList<DeadlineReminder> deduplicated;
    QStringList seenContestUrls;
    for (const DeadlineReminder &reminder : combined) {
        const QString normalizedUrl = reminder.contestUrl.trimmed();
        if (!normalizedUrl.isEmpty() && seenContestUrls.contains(normalizedUrl)) {
            continue;
        }
        if (!normalizedUrl.isEmpty()) {
            seenContestUrls.append(normalizedUrl);
        }
        deduplicated.append(reminder);
    }
    emit remindersUpdated(deduplicated);
}

QDateTime ReminderService::parseDeadline(const QString &text) const
{
    const QRegularExpression regex(
        "(\\d{4})[-/](\\d{1,2})[-/](\\d{1,2})\\s+(\\d{1,2}):(\\d{2})");
    const QRegularExpressionMatch match = regex.match(text);
    if (!match.hasMatch()) {
        return QDateTime();
    }

    const QDate date(
        match.captured(1).toInt(),
        match.captured(2).toInt(),
        match.captured(3).toInt());
    const QTime time(
        match.captured(4).toInt(),
        match.captured(5).toInt());
    return QDateTime(date, time);
}

void ReminderService::finalizeRefresh()
{
    m_loading = false;
    emit loadingChanged(false);
    emitCombinedReminders();
}
