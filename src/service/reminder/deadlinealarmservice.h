#pragma once

#include "service/reminder/reminderservice.h"

#include <QObject>
#include <QSet>

class DeadlineAlarmService : public QObject
{
    Q_OBJECT

public:
    explicit DeadlineAlarmService(QObject *parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const;
    void processReminders(const QList<DeadlineReminder> &reminders);

signals:
    void alarmTriggered(const DeadlineReminder &reminder, int hoursBefore);

private:
    int triggerHoursBefore(const DeadlineReminder &reminder) const;

    QSet<QString> m_triggeredContestUrls;
    bool m_enabled = true;
};
