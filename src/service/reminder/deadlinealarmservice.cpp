#include "deadlinealarmservice.h"

#include <QDateTime>

namespace
{
QString reminderKey(const DeadlineReminder &reminder)
{
    const QString url = reminder.contestUrl.trimmed();
    if (!url.isEmpty()) {
        return url;
    }
    return QString("%1|%2|%3")
        .arg(reminder.courseName, reminder.contestTitle, reminder.deadline.toString(Qt::ISODate));
}
}

DeadlineAlarmService::DeadlineAlarmService(QObject *parent)
    : QObject(parent)
{
}

void DeadlineAlarmService::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool DeadlineAlarmService::isEnabled() const
{
    return m_enabled;
}

void DeadlineAlarmService::processReminders(const QList<DeadlineReminder> &reminders)
{
    if (!m_enabled) {
        return;
    }

    for (const DeadlineReminder &reminder : reminders) {
        const int hoursBefore = triggerHoursBefore(reminder);
        if (hoursBefore <= 0) {
            continue;
        }

        const QString key = QString("%1|%2h").arg(reminderKey(reminder), QString::number(hoursBefore));
        if (m_triggeredContestUrls.contains(key)) {
            continue;
        }

        m_triggeredContestUrls.insert(key);
        emit alarmTriggered(reminder, hoursBefore);
    }
}

int DeadlineAlarmService::triggerHoursBefore(const DeadlineReminder &reminder) const
{
    if (!reminder.deadline.isValid()) {
        return 0;
    }

    const qint64 seconds = QDateTime::currentDateTime().secsTo(reminder.deadline);
    if (seconds < 0) {
        return 0;
    }
    if (seconds < 60 * 60) {
        return 1;
    }
    if (seconds < 2 * 60 * 60) {
        return 2;
    }
    if (seconds < 3 * 60 * 60) {
        return 3;
    }
    return 0;
}
