#include "reminderservice.h"

#include "repository/cache/classcacherepository.h"
#include "repository/submit/classrepository.h"

#include <algorithm>
#include <QRegularExpression>

namespace
{
bool isContestItem(const ContestSetInfo &contestSet)
{
    return contestSet.itemClass.contains("contest-info");
}

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

ReminderService::ReminderService(ClassRepository *classRepository,
                                 ClassCacheRepository *classCacheRepository,
                                 QObject *parent)
    : QObject(parent)
    , m_classRepository(classRepository)
    , m_classCacheRepository(classCacheRepository)
{
    connect(
        m_classRepository,
        &ClassRepository::contestSetsFetched,
        this,
        [this](const ClassPageInfo &classPageInfo,
               const GroupPageInfo &groupPageInfo) {
            if (m_classCacheRepository != nullptr) {
                m_classCacheRepository->saveClass(classPageInfo, groupPageInfo);
            }
            updateClassReminders(
                classPageInfo.classPageUrl,
                collectReminders(classPageInfo, groupPageInfo));
            processNextClass();
        });

    connect(
        m_classRepository,
        &ClassRepository::requestFailed,
        this,
        [this](const QString &, const QString &message) {
            if (!message.isEmpty()) {
                emit failed(message);
            }
            processNextClass();
        });
}

void ReminderService::refreshReminders(const QList<JoinedClassInfo> &classes)
{
    m_pendingClasses.clear();
    m_remindersByClass.clear();
    for (const JoinedClassInfo &joinedClass : classes) {
        m_pendingClasses.enqueue(joinedClass);
    }

    m_loading = true;
    emit loadingChanged(true);
    processNextClass();
}

void ReminderService::processNextClass()
{
    if (m_pendingClasses.isEmpty()) {
        finalizeRefresh();
        return;
    }

    const JoinedClassInfo joinedClass = m_pendingClasses.dequeue();
    if (m_classCacheRepository != nullptr) {
        ClassPageInfo cachedClassInfo;
        GroupPageInfo cachedGroupInfo;
        if (m_classCacheRepository->loadClass(
                joinedClass.url, &cachedClassInfo, &cachedGroupInfo)) {
            updateClassReminders(
                joinedClass.url,
                collectReminders(cachedClassInfo, cachedGroupInfo));
        }
    }

    m_classRepository->fetchContestSets(QUrl(joinedClass.url));
}

QList<DeadlineReminder> ReminderService::collectReminders(
    const ClassPageInfo &classPageInfo,
    const GroupPageInfo &groupPageInfo) const
{
    QList<DeadlineReminder> reminders;
    const QDateTime now = QDateTime::currentDateTime();
    const QUrl groupPageUrl(groupPageInfo.groupPageUrl);
    for (const ContestSetInfo &contestSet : groupPageInfo.contestSets) {
        if (!isContestItem(contestSet)) {
            continue;
        }

        const QDateTime deadline = parseDeadline(contestSet.endtime);
        if (!deadline.isValid() || !isWithinOneWeek(now, deadline)) {
            continue;
        }

        DeadlineReminder reminder;
        reminder.courseName = classPageInfo.courseName;
        reminder.contestTitle = contestSet.title;
        reminder.contestUrl =
            groupPageUrl.resolved(QUrl(contestSet.url)).toString();
        reminder.deadlineText = contestSet.endtime;
        reminder.deadline = deadline;
        reminders << reminder;
    }
    return reminders;
}

void ReminderService::updateClassReminders(const QString &classUrl,
                                           const QList<DeadlineReminder> &reminders)
{
    const auto existingIt = m_remindersByClass.constFind(classUrl);
    if (existingIt != m_remindersByClass.cend()
        && sameReminders(existingIt.value(), reminders)) {
        return;
    }

    m_remindersByClass.insert(classUrl, reminders);
    emitCombinedReminders();
}

void ReminderService::emitCombinedReminders()
{
    QList<DeadlineReminder> combined;
    for (auto it = m_remindersByClass.cbegin(); it != m_remindersByClass.cend(); ++it) {
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
