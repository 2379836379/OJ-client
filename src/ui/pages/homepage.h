#pragma once

#include "parser/UserHomepage.h"
#include "reminder/reminderservice.h"

#include <QWidget>

class ContestPageCache;
class QListWidget;
class PageShell;
class QPushButton;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(ContestPageCache *contestCache,
                      QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showLoggingIn();
    void showLoginFailed(const QString &message);
    void showOpeningHome();
    void showLoadingClasses();
    void showHomeLoadFailed(const QString &message);
    void showClasses(const QList<JoinedClassInfo> &classes);
    void showReminderLoading();
    void showReminders(const QList<DeadlineReminder> &reminders);

signals:
    void classSelected(const QString &name, const QString &url);
    void refreshRequested();
    void favoritesRequested();
    void tasksRequested();
    void statsRequested();
    void reviewRequested();
    void storageRequested();
    void settingsRequested();
    void aiConfigRequested();
    void logoutRequested();
    void themeToggleRequested(bool dark);
    void reminderSelected(const QString &title, const QString &url);

private:
    PageShell *m_shell = nullptr;
    QListWidget *m_classListWidget = nullptr;
    QListWidget *m_reminderListWidget = nullptr;
    QPushButton *m_themeToolButton = nullptr;
    QPushButton *m_collapsedThemeToolButton = nullptr;
    ContestPageCache *m_contestCache = nullptr;
    bool m_darkMode = false;
};
