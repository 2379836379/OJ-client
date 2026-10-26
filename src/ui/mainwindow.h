#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "parser/ProblemPage.h"
#include "reminder/reminderservice.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class OpenJudgeClient;
class CookieStore;
class FavoriteProblemRepository;
class ProblemMetaRepository;
class LoginCache;
class HomepageCache;
class UserHomepageCache;
class GroupHomepageCache;
class GroupPageCache;
class ContestPageCache;
class ProblemPageCache;
class CacheService;
class LoginService;
class EmailVerifyService;
class ApplicationSizeService;
class OpenAiClient;
class AiService;
class FavoriteProblemService;
class ProblemMetaService;
class ResultService;
class SubmitService;
class ReminderService;
class DeadlineAlarmService;
class LoginPage;
class HomePage;
class AiConfigPage;
class ClassPage;
class ContestPage;
class ProblemPage;
class FavoritePage;
class TaskBoardPage;
class StatsPage;
class ReviewPage;
class SettingsPage;
class StoragePage;
class QWidget;
class QAction;
class QMenu;
class QSystemTrayIcon;
class QAudioOutput;
class QMediaPlayer;
class QTimer;

namespace Browse
{
class UserHomepage;
class Group;
class ContestPage;
class ProblemPage;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUiState();
    void connectSignals();
    void setupTrayIcon();
    void ensureStartupVisible();
    void restoreFromTray();
    void showRootPage(QWidget *page);
    void pushPage(QWidget *page);
    void popPage();
    void applyDarkMode(bool dark);
    void setCurrentProblem(const ProblemPageInfo &problemPageInfo);
    void openProblemPage(const QString &problemUrl,
                         const QString &problemTitle,
                         bool preferFavoriteCache);
    void saveCurrentProblemToFavorites();
    bool requiresEmailVerification(const QString &email) const;
    void applyLoginCacheState(const QString &email);
    void showAlarmNotification(const QString &title, const QString &text);
    void playAlarmSound();
    void scheduleNextAlarmCheck();
    void runHourlyAlarmCheck();

private:
    Ui::MainWindow *ui;
    OpenJudgeClient *m_client = nullptr;
    CookieStore *m_cookieStore = nullptr;
    HomepageCache *m_homepageCache = nullptr;
    UserHomepageCache *m_userHomepageCache = nullptr;
    GroupHomepageCache *m_groupHomepageCache = nullptr;
    GroupPageCache *m_groupPageCache = nullptr;
    ContestPageCache *m_contestPageCache = nullptr;
    ProblemPageCache *m_problemPageCache = nullptr;
    LoginCache *m_loginCache = nullptr;
    CacheService *m_cacheService = nullptr;
    LoginService *m_loginService = nullptr;
    EmailVerifyService *m_emailVerifyService = nullptr;
    ApplicationSizeService *m_applicationSizeService = nullptr;
    OpenAiClient *m_openAiClient = nullptr;
    Browse::UserHomepage *m_userHomepage = nullptr;
    Browse::Group *m_browseGroup = nullptr;
    Browse::Group *m_reminderGroup = nullptr;
    Browse::ContestPage *m_browseContestPage = nullptr;
    Browse::ProblemPage *m_browseProblemPage = nullptr;
    AiService *m_aiService = nullptr;
    FavoriteProblemRepository *m_favoriteProblemRepository = nullptr;
    FavoriteProblemService *m_favoriteProblemService = nullptr;
    ProblemMetaRepository *m_problemMetaRepository = nullptr;
    ProblemMetaService *m_problemMetaService = nullptr;
    ResultService *m_resultService = nullptr;
    SubmitService *m_submitService = nullptr;
    ReminderService *m_reminderService = nullptr;
    DeadlineAlarmService *m_deadlineAlarmService = nullptr;
    LoginPage *m_loginPage = nullptr;
    HomePage *m_homePage = nullptr;
    AiConfigPage *m_aiConfigPage = nullptr;
    ClassPage *m_classPage = nullptr;
    ContestPage *m_contestPage = nullptr;
    ProblemPage *m_problemPage = nullptr;
    FavoritePage *m_favoritePage = nullptr;
    TaskBoardPage *m_taskBoardPage = nullptr;
    StatsPage *m_statsPage = nullptr;
    ReviewPage *m_reviewPage = nullptr;
    SettingsPage *m_settingsPage = nullptr;
    StoragePage *m_storagePage = nullptr;
    QVector<int> m_navigationHistory;
    ProblemPageInfo m_currentProblemInfo;
    bool m_hasCurrentProblem = false;
    QString m_verifiedEmail;
    QString m_pendingLoginEmail;
    QString m_pendingLoginPassword;
    QString m_lastTestRequestLog;
    QString m_aiConfigSummary;
    QString m_pendingAiRunTestCallId;
    QString m_pendingAiSubmitCallId;
    bool m_pendingAiSubmitAwaitingResult = false;
    bool m_alarmEnabled = false;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_restoreTrayAction = nullptr;
    QAction *m_exitTrayAction = nullptr;
    QMediaPlayer *m_alarmPlayer = nullptr;
    QAudioOutput *m_alarmAudioOutput = nullptr;
    QTimer *m_alarmCheckTimer = nullptr;
    QList<DeadlineReminder> m_currentReminders;
    QString m_alarmRingPath;
    bool m_allowClose = false;
    bool m_trayNoticeShown = false;
    bool m_darkMode = false;
};

#endif // MAINWINDOW_H
