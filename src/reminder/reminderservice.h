#pragma once

#include "parser/GroupPage.h"
#include "parser/UserHomepage.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QQueue>
#include <QUrl>

namespace Browse
{
class Group;
}

struct DeadlineReminder
{
    QString courseName;
    QString contestTitle;
    QString contestUrl;
    QString deadlineText;
    QDateTime deadline;
};

class ReminderService : public QObject
{
    Q_OBJECT

public:
    explicit ReminderService(Browse::Group *group,
                             QObject *parent = nullptr);

    void refreshReminders(const QList<JoinedClassInfo> &groups);

signals:
    void loadingChanged(bool loading);
    void remindersUpdated(const QList<DeadlineReminder> &reminders);
    void failed(const QString &message);

private:
    void processNextGroup();
    QList<DeadlineReminder> collectReminders(const JoinedClassInfo &group,
                                             const QUrl &groupPageUrl,
                                             const GroupPageInfo &groupPageInfo) const;
    void updateGroupReminders(const QString &groupUrl,
                              const QList<DeadlineReminder> &reminders);
    void emitCombinedReminders();
    QDateTime parseDeadline(const QString &text) const;
    void finalizeRefresh();

    Browse::Group *m_group = nullptr;
    QQueue<JoinedClassInfo> m_pendingGroups;
    JoinedClassInfo m_currentGroup;
    QUrl m_currentGroupPageUrl;
    QHash<QString, QList<DeadlineReminder>> m_remindersByGroup;
    bool m_loading = false;
};
