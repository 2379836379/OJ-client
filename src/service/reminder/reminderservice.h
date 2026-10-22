#pragma once

#include "parser/classparser.h"
#include "parser/groupparser.h"
#include "parser/loginparser.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QQueue>

class ClassRepository;
class ClassCacheRepository;

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
    explicit ReminderService(ClassRepository *classRepository,
                             ClassCacheRepository *classCacheRepository,
                             QObject *parent = nullptr);

    void refreshReminders(const QList<JoinedClassInfo> &classes);

signals:
    void loadingChanged(bool loading);
    void remindersUpdated(const QList<DeadlineReminder> &reminders);
    void failed(const QString &message);

private:
    void processNextClass();
    QList<DeadlineReminder> collectReminders(const ClassPageInfo &classPageInfo,
                                             const GroupPageInfo &groupPageInfo) const;
    void updateClassReminders(const QString &classUrl,
                              const QList<DeadlineReminder> &reminders);
    void emitCombinedReminders();
    QDateTime parseDeadline(const QString &text) const;
    void finalizeRefresh();

    ClassRepository *m_classRepository = nullptr;
    ClassCacheRepository *m_classCacheRepository = nullptr;
    QQueue<JoinedClassInfo> m_pendingClasses;
    QHash<QString, QList<DeadlineReminder>> m_remindersByClass;
    bool m_loading = false;
};
