#pragma once

#include "parser/loginparser.h"
#include "service/reminder/reminderservice.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QFrame;
class QWidget;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
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
    void setToolsExpanded(bool expanded);

    QFrame *m_toolsFrame = nullptr;
    QFrame *m_contentCard = nullptr;
    QListWidget *m_classListWidget = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_favoritesButton = nullptr;
    QPushButton *m_tasksButton = nullptr;
    QPushButton *m_statsButton = nullptr;
    QPushButton *m_reviewButton = nullptr;
    QPushButton *m_storageButton = nullptr;
    QPushButton *m_settingsButton = nullptr;
    QPushButton *m_aiConfigButton = nullptr;
    QPushButton *m_logoutButton = nullptr;
    QPushButton *m_themeButton = nullptr;
    QPushButton *m_collapsedFavoritesButton = nullptr;
    QPushButton *m_collapsedTasksButton = nullptr;
    QPushButton *m_collapsedStatsButton = nullptr;
    QPushButton *m_collapsedReviewButton = nullptr;
    QPushButton *m_collapsedStorageButton = nullptr;
    QPushButton *m_collapsedSettingsButton = nullptr;
    QPushButton *m_collapsedAiConfigButton = nullptr;
    QPushButton *m_collapsedLogoutButton = nullptr;
    QListWidget *m_reminderListWidget = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
