#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config/appconfig.h"
#include "config/apppaths.h"
#include "network/cookiestore.h"
#include "network/openaiclient.h"
#include "network/openjudgeclient.h"
#include "parser/loginparser.h"
#include "repository/cache/classcacherepository.h"
#include "repository/cache/contestcacherepository.h"
#include "repository/cache/homecacherepository.h"
#include "repository/cache/problemcacherepository.h"
#include "repository/favorite/favoriteproblemrepository.h"
#include "repository/meta/problemmetarepository.h"
#include "repository/login/logincacherepository.h"
#include "repository/submit/classrepository.h"
#include "repository/submit/contestrepository.h"
#include "repository/submit/homerepository.h"
#include "repository/submit/problemrepository.h"
#include "repository/submit/resultrepository.h"
#include "repository/submit/submitrepository.h"
#include "service/browse/classservice.h"
#include "service/browse/contestservice.h"
#include "service/browse/homeservice.h"
#include "service/browse/problemservice.h"
#include "service/cache/cacheservice.h"
#include "service/favorite/favoriteproblemservice.h"
#include "service/meta/problemmetaservice.h"
#include "service/app/applicationsizeservice.h"
#include "service/ai/aiservice.h"
#include "service/login/loginservice.h"
#include "service/login/emailverifyservice.h"
#include "service/login/logincacheservice.h"
#include "service/reminder/deadlinealarmservice.h"
#include "service/reminder/reminderservice.h"
#include "service/submit/resultservice.h"
#include "service/submit/submitservice.h"
#include "ui/pages/classpage.h"
#include "ui/pages/contestpage.h"
#include "ui/pages/aiconfigpage.h"
#include "ui/pages/favoritepage.h"
#include "ui/pages/taskboardpage.h"
#include "ui/pages/statspage.h"
#include "ui/pages/reviewpage.h"
#include "ui/pages/homepage.h"
#include "ui/lightmodeiconhelper.h"
#include "ui/pages/loginpage.h"
#include "ui/pages/problempage.h"
#include "ui/pages/settingspage.h"
#include "ui/pages/storagepage.h"

#include <QAction>
#include <QAudioOutput>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMenu>
#include <QMediaPlayer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QJsonParseError>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QWidget>

namespace
{
QIcon loadAppWindowIcon()
{
    QIcon icon(":/images/icon-page-1.ico");
    if (!icon.isNull()) {
        return icon;
    }

    QDir dir(QCoreApplication::applicationDirPath());
    if (dir.dirName().compare("build", Qt::CaseInsensitive) == 0) {
        dir.cdUp();
    }
    const QString path = dir.filePath("images/icon-page-1.ico");
    QIcon fileIcon(path);
    return fileIcon;
}

QString normalizeJudgeLanguage(const QString &languageLabel)
{
    const QString text = languageLabel.trimmed().toLower();
    if (text.contains("python")) {
        return "python";
    }
    if (text.contains("rust") || text == "rs") {
        return "rust";
    }
    if (text == "gcc" || text.startsWith("gcc(") || text.contains("gcc ")
        || text.contains("g++")
        || text.contains("c++") || text.contains("clang++")) {
        return "cpp";
    }
    if (text == "cc" || text == "cxx") {
        return "cpp";
    }
    return text;
}

QString judgeFileNameForLanguage(const QString &language)
{
    if (language == "python" || language == "python3" || language == "py") {
        return "main.py";
    }
    if (language == "rust" || language == "rs") {
        return "main.rs";
    }
    return "main.cpp";
}

QString truncatedText(const QString &text, int limit = 2000)
{
    if (text.size() <= limit) {
        return text;
    }
    return text.left(limit) + "\n...[truncated]";
}

QString jsonValueToText(const QJsonValue &value)
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isBool()) {
        return value.toBool() ? "true" : "false";
    }
    if (value.isDouble()) {
        return QString::number(value.toDouble());
    }
    if (value.isNull() || value.isUndefined()) {
        return "<none>";
    }
    if (value.isArray()) {
        return QString::fromUtf8(
            QJsonDocument(value.toArray()).toJson(QJsonDocument::Indented));
    }
    return QString::fromUtf8(
        QJsonDocument(value.toObject()).toJson(QJsonDocument::Indented));
}

QString formatJudgeResult(const NetworkResult &result)
{
    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    if (!document.isObject()) {
        return QString(
                   "Judge Result\n"
                   "HTTP status: %1\n"
                   "Network OK: %2\n\n"
                   "%3")
            .arg(QString::number(result.statusCode),
                 result.ok ? "true" : "false",
                 QString::fromUtf8(result.body));
    }

    const QJsonObject object = document.object();
    QStringList sections;
    sections << QString("Judge Result\nHTTP status: %1\nNetwork OK: %2")
                    .arg(QString::number(result.statusCode), result.ok ? "true" : "false");

    const QStringList summaryKeys = {"status",
                                     "message",
                                     "exit_code",
                                     "time_ms",
                                     "memory_mb",
                                     "compile_exit_code"};
    QStringList summaryLines;
    for (const QString &key : summaryKeys) {
        if (object.contains(key)) {
            summaryLines << QString("%1: %2").arg(key, jsonValueToText(object.value(key)));
        }
    }
    if (!summaryLines.isEmpty()) {
        sections << summaryLines.join('\n');
    }

    const QStringList outputKeys = {"stdout", "stderr", "compile_stdout", "compile_stderr"};
    for (const QString &key : outputKeys) {
        if (!object.contains(key)) {
            continue;
        }
        const QString value = jsonValueToText(object.value(key));
        if (value.isEmpty() || value == "<none>") {
            continue;
        }
        sections << QString("%1:\n%2").arg(key, value);
    }

    sections << QString("Raw JSON:\n%1")
                    .arg(QString::fromUtf8(document.toJson(QJsonDocument::Indented)));
    return sections.join("\n\n");
}

QString extractJudgeStdout(const NetworkResult &result)
{
    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    if (!document.isObject()) {
        return QString::fromUtf8(result.body);
    }

    const QJsonObject object = document.object();
    if (object.contains("run") && object.value("run").isObject()) {
        const QString stdoutText =
            object.value("run").toObject().value("stdout").toString();
        if (!stdoutText.isNull()) {
            return stdoutText;
        }
    }

    return object.value("stdout").toString();
}

QString extractJudgeDisplayText(const NetworkResult &result)
{
    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    if (!document.isObject()) {
        return QString::fromUtf8(result.body);
    }

    const QJsonObject object = document.object();
    const QJsonObject compileObject = object.value("compile").toObject();
    const QJsonObject runObject = object.value("run").toObject();

    const QString compileStatus = compileObject.value("status").toString();
    const QString compileStderr = compileObject.value("stderr").toString();
    const QString compileStdout = compileObject.value("stdout").toString();
    if (!compileObject.isEmpty()
        && compileStatus.compare("Accepted", Qt::CaseInsensitive) != 0
        && compileStatus.compare("OK", Qt::CaseInsensitive) != 0
        && compileStatus.compare("Success", Qt::CaseInsensitive) != 0) {
        if (!compileStderr.isEmpty()) {
            return compileStderr;
        }
        if (!compileStdout.isEmpty()) {
            return compileStdout;
        }
        if (!compileStatus.isEmpty()) {
            return compileStatus;
        }
    }

    const QString runStatus = runObject.value("status").toString();
    const QString runStdout = runObject.value("stdout").toString();
    const QString runStderr = runObject.value("stderr").toString();
    if (!runObject.isEmpty()) {
        if (!runStatus.isEmpty()
            && runStatus.compare("Accepted", Qt::CaseInsensitive) != 0
            && runStatus.compare("OK", Qt::CaseInsensitive) != 0
            && runStatus.compare("Success", Qt::CaseInsensitive) != 0) {
            if (!runStderr.isEmpty()) {
                return runStderr;
            }
            if (!runStatus.isEmpty()) {
                return runStatus;
            }
        }
        return runStdout;
    }

    const QString stdoutText = extractJudgeStdout(result);
    if (!stdoutText.isEmpty()) {
        return stdoutText;
    }

    const QString stderrText = object.value("stderr").toString();
    if (!stderrText.isEmpty()) {
        return stderrText;
    }

    return QString::fromUtf8(result.body);
}

QString formatJudgeRequest(const QString &language,
                           const QString &fileName,
                           const QString &sourceText,
                           const QString &stdinText)
{
    return QString(
               "Judge Request\n"
               "POST http://10.129.240.62:18080/judge\n"
               "language: %1\n"
               "file: %2\n"
               "time_limit_ms: 2000\n"
               "memory_limit_mb: 256\n"
               "source_length: %3\n"
               "stdin_length: %4\n\n"
               "stdin:\n%5\n\n"
               "source:\n%6")
        .arg(language,
             fileName,
             QString::number(sourceText.size()),
             QString::number(stdinText.size()),
             truncatedText(stdinText),
             truncatedText(sourceText));
}

QString formatSubmitResponse(const NetworkResult &result)
{
    const QJsonDocument document = QJsonDocument::fromJson(result.body);
    QString redirectUrl;
    if (document.isObject()) {
        redirectUrl = document.object().value("redirect").toString();
    }

    QString text = QString(
                       "Submit Response\n"
                       "HTTP status: %1\n"
                       "Network OK: %2\n"
                       "Request URL: %3\n"
                       "Final URL: %4")
                       .arg(QString::number(result.statusCode),
                            result.ok ? "true" : "false",
                            result.requestUrl.toString(),
                            result.finalUrl.toString());
    if (!redirectUrl.isEmpty()) {
        text += "\nRedirect: " + redirectUrl;
    }
    if (!result.body.trimmed().isEmpty()) {
        text += "\n\nRaw JSON:\n"
                + QString::fromUtf8(document.isObject()
                                        ? document.toJson(QJsonDocument::Indented)
                                        : result.body);
    }
    return text;
}

QString formatResultPageSummary(const ResultPageInfo &resultPageInfo)
{
    QString text = QString("Submission ID: %1\nStatus: %2")
                       .arg(resultPageInfo.submissionId.isEmpty()
                                ? QString("<none>")
                                : resultPageInfo.submissionId,
                            resultPageInfo.statusText.isEmpty()
                                ? QString("<none>")
                                : resultPageInfo.statusText);
    if (!resultPageInfo.detailTitle.isEmpty()) {
        text += "\n" + resultPageInfo.detailTitle;
    }
    if (!resultPageInfo.detailText.isEmpty()) {
        text += "\n" + resultPageInfo.detailText;
    }
    return text;
}

void writeStartupLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | " << message << '\n';
}

void clearStartupLog()
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    writeStartupLog("MainWindow: constructor begin");
    ui->setupUi(this);
    writeStartupLog("MainWindow: ui setup complete");
    setMinimumSize(0, 0);
    ui->centralwidget->setMinimumSize(0, 0);
    ui->pageStack->setMinimumSize(0, 0);
    ui->centralwidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->pageStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setWindowTitle(QStringLiteral(" "));
    setWindowFilePath(QString());
    writeStartupLog("MainWindow: window title cleared");
    const QIcon appIcon = loadAppWindowIcon();
    if (!appIcon.isNull()) {
        QApplication::setWindowIcon(appIcon);
        setWindowIcon(appIcon);
    }
    writeStartupLog(QString("MainWindow: window icon applied, null=%1")
                        .arg(windowIcon().isNull() ? "true" : "false"));

    m_client = new OpenJudgeClient(this);
    m_openAiClient = new OpenAiClient(this);
    m_cookieStore = new CookieStore(m_client);
    m_client->setCookieStore(m_cookieStore);
    writeStartupLog("MainWindow: core clients created");

    m_homeRepository = new HomeRepository(m_client, this);
    m_homeCacheRepository = new HomeCacheRepository();
    m_classRepository = new ClassRepository(m_client, this);
    m_classCacheRepository = new ClassCacheRepository();
    m_contestRepository = new ContestRepository(m_client, this);
    m_contestCacheRepository = new ContestCacheRepository();
    m_problemRepository = new ProblemRepository(m_client, this);
    m_problemCacheRepository = new ProblemCacheRepository();
    m_loginCacheRepository = new LoginCacheRepository();
    m_resultRepository = new ResultRepository(m_client, this);
    m_submitRepository = new SubmitRepository(m_client, this);
    m_reminderClassRepository = new ClassRepository(m_client, this);
    writeStartupLog("MainWindow: repositories created");

    m_cacheService = new CacheService(m_homeCacheRepository,
                                      m_classCacheRepository,
                                      m_contestCacheRepository,
                                      m_problemCacheRepository,
                                      this);
    m_loginService = new LoginService(m_client, this);
    m_loginCacheService = new LoginCacheService(m_loginCacheRepository, this);
    m_emailVerifyService = new EmailVerifyService(this);
    m_applicationSizeService = new ApplicationSizeService(this);
    writeStartupLog("MainWindow: app services created");

    const QString openJudgeBaseUrl =
        AppConfig::loadOpenJudgeBaseUrl("http://openjudge.cn");
    const QString judgerBaseUrl =
        AppConfig::loadJudgerBaseUrl("http://10.129.240.62:18080");
    const QString emailVerifyUrl =
        AppConfig::loadEmailVerifyUrl("http://10.129.240.62:8080");
    m_client->setBaseUrl(openJudgeBaseUrl);
    m_client->setJudgerBaseUrl(judgerBaseUrl);
    m_emailVerifyService->setBaseUrl(QUrl(emailVerifyUrl));
    writeStartupLog("MainWindow: service endpoints configured");
    const OpenAiConfig openAiConfig = AppConfig::loadOpenAiConfig();
    writeStartupLog("MainWindow: openai config loaded");
    m_openAiClient->setBaseUrl(openAiConfig.baseUrl);
    m_aiService = new AiService(m_openAiClient, this);
    m_aiService->setConfig(openAiConfig);
    m_aiService->setToolContext(AiToolContext{
        [this]() { return m_problemPage->currentProblemDetailText(); },
        [this]() {
            const QString label = m_problemPage->currentLanguageLabel();
            const QString value = m_problemPage->currentLanguageValue();
            if (label.isEmpty() && value.isEmpty()) {
                return QString();
            }
            return QString("label: %1\nvalue: %2")
                .arg(label.isEmpty() ? "<none>" : label,
                     value.isEmpty() ? "<none>" : value);
        },
        [this]() { return m_problemPage->currentSourceCode(); },
        [this]() { return m_problemPage->currentTestInput(); },
        [this]() { return m_problemPage->currentTestOutput(); },
        [this](const QString &text) { m_problemPage->setSourceCodeText(text); },
        [this](const QString &text) { m_problemPage->setTestInputText(text); },
        [this](const QString &callId, const QString &stdinText) {
            m_pendingAiRunTestCallId = callId;
            const QString language =
                normalizeJudgeLanguage(m_problemPage->currentLanguageLabel());
            const QString sourceText = m_problemPage->currentSourceCode();
            if (language.isEmpty()) {
                m_aiService->failToolCall(callId, "No language selected.");
                return;
            }
            if (sourceText.trimmed().isEmpty()) {
                m_aiService->failToolCall(callId, "Source code is empty.");
                return;
            }

            m_problemPage->setTestInputText(stdinText);
            m_lastTestRequestLog = formatJudgeRequest(
                language,
                judgeFileNameForLanguage(language),
                sourceText,
                stdinText);
            m_problemPage->showTesting(true);
            m_problemPage->showTestResult("Running test...");
            m_client->judgeSource(language,
                                  judgeFileNameForLanguage(language),
                                  sourceText.toUtf8(),
                                  stdinText.toUtf8());
        },
        [this](const QString &callId) {
            m_pendingAiSubmitCallId = callId;
            m_pendingAiSubmitAwaitingResult = false;
            const QString languageValue = m_problemPage->currentLanguageValue();
            const QString sourceText = m_problemPage->currentSourceCode();
            if (languageValue.trimmed().isEmpty()) {
                m_aiService->failToolCall(callId, "No language selected.");
                return;
            }
            if (sourceText.trimmed().isEmpty()) {
                m_aiService->failToolCall(callId, "Source code is empty.");
                return;
            }
            m_submitService->submitSolution(languageValue, sourceText);
        }});
    writeStartupLog("MainWindow: ai service configured");
    m_aiConfigSummary = QString("Config: %1 | model: %2")
                            .arg(openAiConfig.sourcePath.isEmpty()
                                     ? QString("defaults")
                                     : openAiConfig.sourcePath,
                                 openAiConfig.model);
    m_alarmEnabled = AppConfig::loadAlarmEnabled(false);
    m_alarmRingPath = AppConfig::loadRingPath();
    m_homeService = new HomeService(m_homeRepository, m_homeCacheRepository, this);
    m_classService = new ClassService(
        m_classRepository, m_classCacheRepository, this);
    m_contestService = new ContestService(
        m_contestRepository, m_contestCacheRepository, this);
    m_problemService = new ProblemService(
        m_problemRepository, m_problemCacheRepository, this);
    m_favoriteProblemRepository = new FavoriteProblemRepository();
    m_favoriteProblemService = new FavoriteProblemService(
        m_favoriteProblemRepository, this);
    m_problemMetaRepository = new ProblemMetaRepository();
    m_problemMetaService = new ProblemMetaService(
        m_problemMetaRepository, this);
    m_resultService = new ResultService(m_resultRepository, this);
    m_submitService = new SubmitService(m_submitRepository, this);
    m_reminderService = new ReminderService(
        m_reminderClassRepository,
        m_classCacheRepository,
        this);
    m_deadlineAlarmService = new DeadlineAlarmService(this);
    m_deadlineAlarmService->setEnabled(m_alarmEnabled);
    m_alarmAudioOutput = new QAudioOutput(this);
    m_alarmAudioOutput->setVolume(1.0f);
    m_alarmPlayer = new QMediaPlayer(this);
    m_alarmPlayer->setAudioOutput(m_alarmAudioOutput);
    m_alarmCheckTimer = new QTimer(this);
    m_alarmCheckTimer->setSingleShot(true);
    writeStartupLog("MainWindow: domain services created");

    setupUiState();
    writeStartupLog("MainWindow: setupUiState complete");
    setupTrayIcon();
    writeStartupLog("MainWindow: setupTrayIcon complete");
    connectSignals();
    writeStartupLog("MainWindow: connectSignals complete");
    connect(m_alarmCheckTimer, &QTimer::timeout, this, [this]() {
        runHourlyAlarmCheck();
        scheduleNextAlarmCheck();
    });
    scheduleNextAlarmCheck();
    ensureStartupVisible();
    writeStartupLog("MainWindow: ensureStartupVisible complete");
    QTimer::singleShot(0, this, [this]() {
        writeStartupLog("MainWindow: deferred initialization begin");
        if (m_loginCacheService->initialize()) {
            CachedLoginInfo loginInfo;
            if (m_loginCacheService->loadLastLogin(&loginInfo)) {
                m_loginPage->setCredentials(loginInfo.email, loginInfo.password);
            }
        }
        applyLoginCacheState(m_loginPage->email());

        if (!m_favoriteProblemService->initialize()) {
        }
        if (!m_problemMetaService->initialize()) {
        }
        writeStartupLog("MainWindow: deferred initialization complete");
    });
    writeStartupLog("MainWindow: constructor end");
}

MainWindow::~MainWindow()
{
    delete m_homeCacheRepository;
    delete m_classCacheRepository;
    delete m_contestCacheRepository;
    delete m_problemCacheRepository;
    delete m_loginCacheRepository;
    delete m_favoriteProblemRepository;
    delete m_problemMetaRepository;
    delete ui;
}

void MainWindow::setupUiState()
{
    writeStartupLog("MainWindow: setupUiState begin");
    m_loginPage = new LoginPage(this);
    writeStartupLog("MainWindow: login page created");
    m_homePage = new HomePage(this);
    writeStartupLog("MainWindow: home page created");
    m_aiConfigPage = new AiConfigPage(this);
    writeStartupLog("MainWindow: ai config page created");
    m_classPage = new ClassPage(this);
    writeStartupLog("MainWindow: class page created");
    m_contestPage = new ContestPage(this);
    writeStartupLog("MainWindow: contest page created");
    m_problemPage = new ProblemPage(this);
    writeStartupLog("MainWindow: problem page created");
    m_favoritePage = new FavoritePage(this);
    writeStartupLog("MainWindow: favorite page created");
    m_taskBoardPage = new TaskBoardPage(this);
    writeStartupLog("MainWindow: task board page created");
    m_statsPage = new StatsPage(this);
    writeStartupLog("MainWindow: stats page created");
    m_reviewPage = new ReviewPage(this);
    writeStartupLog("MainWindow: review page created");
    m_storagePage = new StoragePage(this);
    writeStartupLog("MainWindow: storage page created");
    m_settingsPage = new SettingsPage(this);
    writeStartupLog("MainWindow: settings page created");

    ui->pageStack->addWidget(m_loginPage);
    ui->pageStack->addWidget(m_homePage);
    ui->pageStack->addWidget(m_aiConfigPage);
    ui->pageStack->addWidget(m_classPage);
    ui->pageStack->addWidget(m_contestPage);
    ui->pageStack->addWidget(m_problemPage);
    ui->pageStack->addWidget(m_favoritePage);
    ui->pageStack->addWidget(m_taskBoardPage);
    ui->pageStack->addWidget(m_statsPage);
    ui->pageStack->addWidget(m_reviewPage);
    ui->pageStack->addWidget(m_storagePage);
    ui->pageStack->addWidget(m_settingsPage);
    writeStartupLog("MainWindow: pages added to stack");

    m_problemPage->setAiConfigSummary(m_aiConfigSummary);
    writeStartupLog("MainWindow: problem page ai summary set");
    QString configPath;
    m_aiConfigPage->setConfigText(configPath = m_aiService->config().sourcePath,
                                  AppConfig::loadConfigText(&configPath));
    writeStartupLog("MainWindow: ai config text loaded");
    m_storagePage->setAlarmEnabled(m_alarmEnabled);
    m_storagePage->setRingPath(m_alarmRingPath);

    showRootPage(m_loginPage);
    writeStartupLog("MainWindow: setupUiState end");
}

void MainWindow::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    QIcon trayIcon = loadAppWindowIcon();
    if (trayIcon.isNull()) {
        trayIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
    }
    m_trayIcon->setIcon(trayIcon);
    writeStartupLog(QString("MainWindow: tray icon applied, null=%1")
                        .arg(trayIcon.isNull() ? "true" : "false"));
    m_trayIcon->setToolTip("oj-client");

    m_trayMenu = new QMenu(this);
    m_restoreTrayAction = m_trayMenu->addAction("Restore");
    m_exitTrayAction = m_trayMenu->addAction("Exit");
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_restoreTrayAction, &QAction::triggered, this, [this]() {
        restoreFromTray();
    });
    connect(m_exitTrayAction, &QAction::triggered, this, [this]() {
        m_allowClose = true;
        if (m_trayIcon != nullptr) {
            m_trayIcon->hide();
        }
        qApp->quit();
    });
    connect(
        m_trayIcon,
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                restoreFromTray();
            }
        });

    m_trayIcon->show();
}

void MainWindow::ensureStartupVisible()
{
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    showNormal();
    raise();
    activateWindow();

    QTimer::singleShot(0, this, [this]() {
        setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        showNormal();
        raise();
        activateWindow();
    });
}

void MainWindow::connectSignals()
{
    connect(
        m_loginPage,
        &LoginPage::loginRequested,
        this,
        [this](const QString &email, const QString &password) {
            if (email.isEmpty() || password.isEmpty()) {
                m_loginPage->showLoginFailed("Email and password are required.");
                return;
            }

            if (requiresEmailVerification(email)) {
                const QString code = m_loginPage->verificationCode();
                if (code.isEmpty()) {
                    m_loginPage->showLoginFailed(
                        "Verification code is required for a new email.");
                    return;
                }

                m_pendingLoginEmail = email;
                m_pendingLoginPassword = password;
                m_emailVerifyService->verifyCode(email, code);
                return;
            }

            m_loginService->login(email, password);
        });
    connect(
        m_loginPage,
        &LoginPage::emailEdited,
        this,
        [this](const QString &email) {
            applyLoginCacheState(email);
        });
    connect(
        m_loginPage,
        &LoginPage::verificationRequested,
        this,
        [this](const QString &email) {
            if (email.isEmpty()) {
                m_loginPage->showLoginFailed("Email is required.");
                return;
            }

            if (!requiresEmailVerification(email)) {
                m_loginPage->showVerificationMessage(
                    "This email is already cached. Verification is not required.");
                return;
            }

            m_emailVerifyService->sendCode(email);
        });
    connect(
        m_loginService,
        &LoginService::loggingChanged,
        this,
        [this](bool logging) {
            if (logging) {
                m_loginPage->showLoggingIn();
            } else if (ui->pageStack->currentWidget() == m_loginPage) {
                m_loginPage->showIdle();
            }
        });
    connect(
        m_loginService,
        &LoginService::failed,
        this,
        [this](const QString &message) {
            showRootPage(m_loginPage);
            m_loginPage->showLoginFailed(message);
        });
    connect(
        m_loginService,
        &LoginService::loginSucceeded,
        this,
        [this](const QUrl &personalHomeUrl) {
            m_loginCacheService->saveLogin(
                m_loginPage->email(),
                m_loginPage->password());
            m_verifiedEmail = m_loginPage->email();
            showRootPage(m_homePage);
            m_homePage->showOpeningHome();
            m_homeService->openHome(personalHomeUrl);
        });
    connect(
        m_loginCacheService,
        &LoginCacheService::failed,
        this,
        [this](const QString &message) {
            if (ui->pageStack->currentWidget() == m_loginPage) {
                m_loginPage->showLoginFailed(message);
            }
        });
    connect(
        m_emailVerifyService,
        &EmailVerifyService::sendingChanged,
        this,
        [this](bool sending) {
            m_loginPage->showSendingVerification(sending);
            if (!sending) {
                m_loginPage->showIdle();
            }
        });
    connect(
        m_emailVerifyService,
        &EmailVerifyService::verifyingChanged,
        this,
        [this](bool verifying) {
            m_loginPage->showVerifyingCode(verifying);
            if (!verifying) {
                m_loginPage->showIdle();
            }
        });
    connect(
        m_emailVerifyService,
        &EmailVerifyService::codeSent,
        this,
        [this](const QString &message) {
            m_loginPage->showVerificationMessage(message);
        });
    connect(
        m_emailVerifyService,
        &EmailVerifyService::verified,
        this,
        [this](const QString &email) {
            m_verifiedEmail = email.trimmed();
            m_loginPage->showVerificationMessage(
                "Verification succeeded. Logging in...");
            m_loginService->login(m_pendingLoginEmail, m_pendingLoginPassword);
        });
    connect(
        m_emailVerifyService,
        &EmailVerifyService::failed,
        this,
        [this](const QString &message) {
            m_loginPage->showLoginFailed(message);
            m_pendingLoginEmail.clear();
            m_pendingLoginPassword.clear();
        });

    connect(
        m_homePage,
        &HomePage::themeToggleRequested,
        this,
        [this](bool dark) {
            applyDarkMode(dark);
        });
    connect(m_classPage, &ClassPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_contestPage, &ContestPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_problemPage, &ProblemPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_favoritePage, &FavoritePage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_storagePage, &StoragePage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_aiConfigPage, &AiConfigPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_settingsPage, &SettingsPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(
        m_homePage,
        &HomePage::classSelected,
        this,
        [this](const QString &name, const QString &url) {
            m_classPage->openClass(name, url);
            pushPage(m_classPage);
            m_classService->openClass(QUrl(url));
        });
    connect(
        m_homePage,
        &HomePage::favoritesRequested,
        this,
        [this]() {
            m_favoritePage->showFoldersUnavailable();
            m_favoritePage->showFolders(
                m_favoriteProblemService->loadFolders());
            pushPage(m_favoritePage);
        });
    connect(
        m_homePage,
        &HomePage::tasksRequested,
        this,
        [this]() {
            m_taskBoardPage->showTasks(m_problemMetaService->loadAllMeta());
            pushPage(m_taskBoardPage);
        });
    connect(
        m_homePage,
        &HomePage::statsRequested,
        this,
        [this]() {
            m_statsPage->showStats(
                m_problemMetaService->statusCounts(),
                m_problemMetaService->tagCounts(),
                m_problemMetaService->notesCount(),
                m_favoriteProblemService->loadFolders(),
                m_problemMetaService->reviewProblems());
            pushPage(m_statsPage);
        });
    connect(
        m_homePage,
        &HomePage::reviewRequested,
        this,
        [this]() {
            m_reviewPage->showDue(m_problemMetaService->dueReviewProblems());
            pushPage(m_reviewPage);
        });
    connect(
        m_homePage,
        &HomePage::refreshRequested,
        this,
        [this]() {
            const QUrl homeUrl = m_homeService->currentHomeUrl();
            if (!homeUrl.isValid() || homeUrl.scheme().isEmpty()) {
                return;
            }
            m_homePage->showOpeningHome();
            m_homeService->openHome(homeUrl);
        });
    connect(
        m_homePage,
        &HomePage::storageRequested,
        this,
        [this]() {
            m_storagePage->showSizes(
                m_cacheService->formattedTotalCacheSize(),
                m_applicationSizeService->formattedTotalApplicationSize());
            pushPage(m_storagePage);
        });
    connect(
        m_homePage,
        &HomePage::aiConfigRequested,
        this,
        [this]() {
            QString configPath;
            const QString configText = AppConfig::loadConfigText(&configPath);
            m_aiConfigPage->setConfigText(configPath, configText);
            pushPage(m_aiConfigPage);
        });
    connect(
        m_homePage,
        &HomePage::settingsRequested,
        this,
        [this]() {
            m_settingsPage->setUrls(
                AppConfig::loadOpenJudgeBaseUrl("http://openjudge.cn"),
                AppConfig::loadJudgerBaseUrl("http://10.129.240.62:18080"),
                AppConfig::loadEmailVerifyUrl("http://10.129.240.62:8080"));
            pushPage(m_settingsPage);
        });
    connect(
        m_homePage,
        &HomePage::logoutRequested,
        this,
        [this]() {
            m_client->logout();
            showRootPage(m_loginPage);
            m_loginPage->showIdle();
        });
    connect(
        m_homePage,
        &HomePage::reminderSelected,
        this,
        [this](const QString &title, const QString &url) {
            const QUrl contestUrl(url);
            if (!contestUrl.isValid() || contestUrl.scheme().isEmpty()) {
                return;
            }

            m_contestPage->openContest(title, contestUrl.toString());
            pushPage(m_contestPage);
            m_contestService->openContest(contestUrl);
        });

    connect(m_classPage, &ClassPage::backRequested, this, &MainWindow::popPage);
    connect(m_classPage, &ClassPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_classPage,
        &ClassPage::contestSelected,
        this,
        [this](const QString &title, const QString &url) {
            m_contestPage->openContest(title, url);
            pushPage(m_contestPage);
            m_contestService->openContest(QUrl(url));
        });

    connect(m_contestPage, &ContestPage::backRequested, this, &MainWindow::popPage);
    connect(m_contestPage, &ContestPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_contestPage,
        &ContestPage::problemSelected,
        this,
        [this](const QString &title, const QString &url) {
            openProblemPage(url, title, true);
        });

    connect(m_problemPage, &ProblemPage::backRequested, this, &MainWindow::popPage);
    connect(m_problemPage, &ProblemPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_problemPage,
        &ProblemPage::favoriteRequested,
        this,
        &MainWindow::saveCurrentProblemToFavorites);
    connect(
        m_problemPage,
        &ProblemPage::saveProblemMetaRequested,
        this,
        [this](const ProblemMeta &meta) {
            if (!m_problemMetaService->saveMeta(meta)) {
                m_problemPage->showProblemLoadFailed(
                    m_problemMetaService->lastError());
            }
        });
    connect(
        m_problemPage,
        &ProblemPage::aiAskRequested,
        this,
        [this](const QString &question) {
            m_aiService->ask(question,
                             m_problemPage->currentProblemDetailText(),
                             m_problemPage->currentSourceCode(),
                             m_problemPage->currentTestInput(),
                             m_problemPage->currentTestOutput());
        });
    connect(
        m_problemPage,
        &ProblemPage::testRequested,
        this,
        [this](const QString &languageLabel,
               const QString &sourceText,
               const QString &stdinText) {
            const QString language = normalizeJudgeLanguage(languageLabel);
            if (language.isEmpty()) {
                m_problemPage->showTestFailed("No language selected.");
                return;
            }
            if (sourceText.trimmed().isEmpty()) {
                m_problemPage->showTestFailed("Source code is empty.");
                return;
            }

            m_lastTestRequestLog = formatJudgeRequest(
                language,
                judgeFileNameForLanguage(language),
                sourceText,
                stdinText);
            m_problemPage->showTesting(true);
            m_problemPage->showTestResult("Running test...");
            m_client->judgeSource(language,
                                  judgeFileNameForLanguage(language),
                                  sourceText.toUtf8(),
                                  stdinText.toUtf8());
        });
    connect(
        m_problemPage,
        &ProblemPage::submitRequested,
        this,
        [this](const QString &language, const QString &sourceText) {
            if (language.isEmpty()) {
                m_problemPage->showMissingLanguage();
                return;
            }
            m_submitService->submitSolution(language, sourceText);
        });

    connect(m_favoritePage, &FavoritePage::backRequested, this, &MainWindow::popPage);
    connect(m_favoritePage, &FavoritePage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_favoritePage,
        &FavoritePage::refreshRequested,
        this,
        [this]() {
            m_favoritePage->showFolders(
                m_favoriteProblemService->loadFolders());
        });
    connect(
        m_favoritePage,
        &FavoritePage::exportRequested,
        this,
        [this]() {
            const QString path = QFileDialog::getSaveFileName(
                this,
                "Export Favorites",
                "favorites.json",
                "JSON Files (*.json)");
            if (path.isEmpty()) {
                return;
            }

            QJsonArray foldersArray;
            const QList<FavoriteFolderInfo> folders =
                m_favoriteProblemService->loadFolders();
            for (const FavoriteFolderInfo &folder : folders) {
                QJsonObject folderObject;
                folderObject.insert("name", folder.name);
                QJsonArray problemsArray;
                const QList<ProblemPageInfo> favorites =
                    m_favoriteProblemService->loadFavoritesInFolder(folder.id);
                for (const ProblemPageInfo &info : favorites) {
                    QJsonObject problemObject;
                    problemObject.insert("problemUrl", info.problemUrl);
                    problemObject.insert("title", info.title);
                    problemObject.insert("submitUrl", info.submitUrl);
                    problemObject.insert("timeLimit", info.timeLimit);
                    problemObject.insert("memoryLimit", info.memoryLimit);
                    problemObject.insert("description", info.description);
                    problemObject.insert("starterCode", info.starterCode);
                    problemObject.insert("inputSpec", info.inputSpec);
                    problemObject.insert("outputSpec", info.outputSpec);
                    problemObject.insert("sampleInput", info.sampleInput);
                    problemObject.insert("sampleOutput", info.sampleOutput);
                    problemObject.insert("hint", info.hint);
                    problemsArray.append(problemObject);
                }
                folderObject.insert("problems", problemsArray);
                foldersArray.append(folderObject);
            }

            QJsonObject root;
            root.insert("version", 1);
            root.insert("folders", foldersArray);

            QFile file(path);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                m_favoritePage->showFavoriteOperationFailed(
                    "Failed to write file: " + file.errorString());
                return;
            }
            file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
            file.close();
            m_favoritePage->showFavoriteOperationFailed(
                QString("Exported %1 folder(s) to %2")
                    .arg(QString::number(folders.size()), path));
        });
    connect(
        m_favoritePage,
        &FavoritePage::importRequested,
        this,
        [this]() {
            const QString path = QFileDialog::getOpenFileName(
                this,
                "Import Favorites",
                QString(),
                "JSON Files (*.json)");
            if (path.isEmpty()) {
                return;
            }

            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                m_favoritePage->showFavoriteOperationFailed(
                    "Failed to read file: " + file.errorString());
                return;
            }
            const QByteArray data = file.readAll();
            file.close();

            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                m_favoritePage->showFavoriteOperationFailed(
                    "Invalid JSON: " + parseError.errorString());
                return;
            }

            const QJsonArray foldersArray =
                document.object().value("folders").toArray();

            QHash<QString, qint64> folderIdByName;
            for (const FavoriteFolderInfo &folder :
                 m_favoriteProblemService->loadFolders()) {
                folderIdByName.insert(folder.name, folder.id);
            }

            int importedProblems = 0;
            for (const QJsonValue &folderValue : foldersArray) {
                const QJsonObject folderObject = folderValue.toObject();
                const QString folderName = folderObject.value("name").toString().trimmed();
                if (folderName.isEmpty()) {
                    continue;
                }

                qint64 folderId = -1;
                if (folderIdByName.contains(folderName)) {
                    folderId = folderIdByName.value(folderName);
                } else {
                    if (!m_favoriteProblemService->createFolder(folderName, &folderId)) {
                        m_favoritePage->showFavoriteOperationFailed(
                            m_favoriteProblemService->lastError());
                        return;
                    }
                    if (folderId < 0) {
                        for (const FavoriteFolderInfo &folder :
                             m_favoriteProblemService->loadFolders()) {
                            if (folder.name == folderName) {
                                folderId = folder.id;
                                break;
                            }
                        }
                    }
                    folderIdByName.insert(folderName, folderId);
                }

                if (folderId < 0) {
                    continue;
                }

                const QJsonArray problemsArray =
                    folderObject.value("problems").toArray();
                for (const QJsonValue &problemValue : problemsArray) {
                    const QJsonObject problemObject = problemValue.toObject();
                    ProblemPageInfo info;
                    info.problemUrl = problemObject.value("problemUrl").toString();
                    if (info.problemUrl.isEmpty()) {
                        continue;
                    }
                    info.title = problemObject.value("title").toString();
                    info.submitUrl = problemObject.value("submitUrl").toString();
                    info.timeLimit = problemObject.value("timeLimit").toString();
                    info.memoryLimit = problemObject.value("memoryLimit").toString();
                    info.description = problemObject.value("description").toString();
                    info.starterCode = problemObject.value("starterCode").toString();
                    info.inputSpec = problemObject.value("inputSpec").toString();
                    info.outputSpec = problemObject.value("outputSpec").toString();
                    info.sampleInput = problemObject.value("sampleInput").toString();
                    info.sampleOutput = problemObject.value("sampleOutput").toString();
                    info.hint = problemObject.value("hint").toString();
                    info.tried_people = 0;
                    info.passed_people = 0;
                    if (m_favoriteProblemService->saveFavoriteToFolder(info, folderId)) {
                        ++importedProblems;
                    }
                }
            }

            m_favoritePage->showFolders(m_favoriteProblemService->loadFolders());
            m_favoritePage->showFavoriteOperationFailed(
                QString("Imported %1 problem(s).")
                    .arg(QString::number(importedProblems)));
        });
    connect(
        m_favoritePage,
        &FavoritePage::createFolderRequested,
        this,
        [this](const QString &folderName) {
            if (!m_favoriteProblemService->createFolder(folderName)) {
                m_favoritePage->showFavoriteOperationFailed(
                    m_favoriteProblemService->lastError());
                return;
            }

            m_favoritePage->showFolders(
                m_favoriteProblemService->loadFolders());
        });
    connect(
        m_favoritePage,
        &FavoritePage::folderRemoveRequested,
        this,
        [this](qint64 folderId, const QString &) {
            if (!m_favoriteProblemService->removeFolder(folderId)) {
                m_favoritePage->showFavoriteOperationFailed(
                    m_favoriteProblemService->lastError());
                return;
            }

            m_favoritePage->showFolders(
                m_favoriteProblemService->loadFolders());
        });
    connect(
        m_favoritePage,
        &FavoritePage::folderSelected,
        this,
        [this](qint64 folderId, const QString &folderName) {
            m_favoritePage->showFavorites(
                folderId,
                folderName,
                m_favoriteProblemService->loadFavoritesInFolder(folderId));
        });
    connect(
        m_favoritePage,
        &FavoritePage::favoriteSelected,
        this,
        [this](const QString &title, const QString &url) {
            openProblemPage(url, title, true);
        });
    connect(
        m_favoritePage,
        &FavoritePage::favoriteRemoveRequested,
        this,
        [this](qint64 folderId,
               const QString &folderName,
               const QString &problemUrl) {
            if (!m_favoriteProblemService->removeFavoriteFromFolder(problemUrl, folderId)) {
                m_favoritePage->showFavoriteOperationFailed(
                    m_favoriteProblemService->lastError());
                return;
            }

            m_favoritePage->showFavorites(
                folderId,
                folderName,
                m_favoriteProblemService->loadFavoritesInFolder(folderId));
        });
    connect(m_taskBoardPage, &TaskBoardPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(m_taskBoardPage, &TaskBoardPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_taskBoardPage, &TaskBoardPage::refreshRequested, this, [this]() {
        m_taskBoardPage->showTasks(m_problemMetaService->loadAllMeta());
    });
    connect(
        m_taskBoardPage,
        &TaskBoardPage::problemSelected,
        this,
        [this](const QString &title, const QString &url) {
            openProblemPage(url, title, true);
        });
    connect(
        m_taskBoardPage,
        &TaskBoardPage::statusChangeRequested,
        this,
        [this](const QString &url, const QString &newStatus) {
            if (url.isEmpty()) {
                return;
            }
            ProblemMeta meta;
            meta.problemUrl = url;
            m_problemMetaService->loadMeta(url, &meta);
            meta.taskStatus = newStatus;
            if (!m_problemMetaService->saveMeta(meta)) {
                return;
            }
            m_taskBoardPage->showTasks(m_problemMetaService->loadAllMeta());
        });
    connect(m_statsPage, &StatsPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(m_statsPage, &StatsPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_statsPage, &StatsPage::refreshRequested, this, [this]() {
        m_statsPage->showStats(
            m_problemMetaService->statusCounts(),
            m_problemMetaService->tagCounts(),
            m_problemMetaService->notesCount(),
            m_favoriteProblemService->loadFolders(),
            m_problemMetaService->reviewProblems());
    });
    connect(
        m_statsPage,
        &StatsPage::problemSelected,
        this,
        [this](const QString &title, const QString &url) {
            openProblemPage(url, title, true);
        });
    connect(m_reviewPage, &ReviewPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(m_reviewPage, &ReviewPage::themeToggleRequested, this, [this](bool dark) {
        applyDarkMode(dark);
    });
    connect(m_reviewPage, &ReviewPage::refreshRequested, this, [this]() {
        m_reviewPage->showDue(m_problemMetaService->dueReviewProblems());
    });
    connect(
        m_reviewPage,
        &ReviewPage::problemSelected,
        this,
        [this](const QString &title, const QString &url) {
            openProblemPage(url, title, true);
        });
    connect(
        m_reviewPage,
        &ReviewPage::gradeRequested,
        this,
        [this](const QString &url, int grade) {
            if (url.isEmpty()) {
                return;
            }
            m_problemMetaService->gradeReview(url, grade);
            m_reviewPage->showDue(m_problemMetaService->dueReviewProblems());
        });
    connect(m_storagePage, &StoragePage::backRequested, this, &MainWindow::popPage);
    connect(m_storagePage, &StoragePage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(m_aiConfigPage, &AiConfigPage::backRequested, this, &MainWindow::popPage);
    connect(m_aiConfigPage, &AiConfigPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_aiConfigPage,
        &AiConfigPage::saveRequested,
        this,
        [this](const QString &content) {
            QString errorMessage;
            QString savedPath;
            if (!AppConfig::saveConfigText(content, &savedPath, &errorMessage)) {
                m_aiConfigPage->showSaveFailed(errorMessage);
                return;
            }
            OpenAiConfig savedConfig = AppConfig::loadOpenAiConfig();
            m_openAiClient->setBaseUrl(savedConfig.baseUrl);
            m_aiService->setConfig(savedConfig);
            m_aiConfigSummary = QString("Config: %1 | model: %2")
                                    .arg(savedConfig.sourcePath.isEmpty()
                                             ? QString("defaults")
                                             : savedConfig.sourcePath,
                                         savedConfig.model);
            m_problemPage->setAiConfigSummary(m_aiConfigSummary);
            m_aiConfigPage->showSaveSucceeded(savedPath);
        });
    connect(m_settingsPage, &SettingsPage::backRequested, this, &MainWindow::popPage);
    connect(m_settingsPage, &SettingsPage::homeRequested, this, [this]() {
        showRootPage(m_homePage);
    });
    connect(
        m_settingsPage,
        &SettingsPage::saveRequested,
        this,
        [this](const QString &openJudgeUrl,
               const QString &judgerUrl,
               const QString &emailVerifyUrl) {
            QString errorMessage;
            if (!AppConfig::saveOpenJudgeBaseUrl(openJudgeUrl, &errorMessage)
                || !AppConfig::saveJudgerBaseUrl(judgerUrl, &errorMessage)
                || !AppConfig::saveEmailVerifyUrl(emailVerifyUrl, &errorMessage)) {
                m_settingsPage->showSaveFailed(errorMessage);
                return;
            }
            // Live apply; empty fields fall back to the same defaults used at startup.
            const QString effectiveOpenJudge =
                openJudgeUrl.isEmpty() ? QStringLiteral("http://openjudge.cn") : openJudgeUrl;
            const QString effectiveJudger =
                judgerUrl.isEmpty() ? QStringLiteral("http://10.129.240.62:18080") : judgerUrl;
            const QString effectiveEmailVerify =
                emailVerifyUrl.isEmpty() ? QStringLiteral("http://10.129.240.62:8080") : emailVerifyUrl;
            m_client->setBaseUrl(effectiveOpenJudge);
            m_client->setJudgerBaseUrl(effectiveJudger);
            m_emailVerifyService->setBaseUrl(QUrl(effectiveEmailVerify));
            m_settingsPage->showSaveSucceeded();
        });
    connect(
        m_storagePage,
        &StoragePage::clearCacheRequested,
        this,
        [this]() {
            m_storagePage->showClearing(true);
            if (!m_cacheService->clearAllCaches()) {
                m_storagePage->showOperationFailed(m_cacheService->lastError());
                return;
            }

            m_storagePage->showClearSucceeded(
                m_cacheService->formattedTotalCacheSize(),
                m_applicationSizeService->formattedTotalApplicationSize());
        });
    connect(
        m_storagePage,
        &StoragePage::alarmToggled,
        this,
        [this](bool enabled) {
            m_alarmEnabled = enabled;
            if (m_deadlineAlarmService != nullptr) {
                m_deadlineAlarmService->setEnabled(enabled);
                if (enabled) {
                    m_deadlineAlarmService->processReminders(m_currentReminders);
                }
            }
            QString errorMessage;
            if (!AppConfig::saveAlarmEnabled(enabled, &errorMessage)) {
                m_storagePage->showOperationFailed(errorMessage);
            }
        });
    connect(
        m_storagePage,
        &StoragePage::alarmTestRequested,
        this,
        [this]() {
            showAlarmNotification("Contest deadline alarm",
                                  "Alarm test\nThis is a manual test notification.");
            playAlarmSound();
        });
    connect(
        m_storagePage,
        &StoragePage::ringPathPickRequested,
        this,
        [this]() {
            const QString path = QFileDialog::getOpenFileName(
                this,
                "Choose Ring Audio",
                QString(),
                "Audio Files (*.wav *.mp3 *.ogg *.flac *.m4a);;All Files (*)");
            if (!path.isEmpty()) {
                m_alarmRingPath = path;
                m_storagePage->setRingPath(path);
                QString errorMessage;
                if (!AppConfig::saveRingPath(path, &errorMessage)) {
                    m_storagePage->showOperationFailed(errorMessage);
                }
            }
        });

    connect(
        m_aiService,
        &AiService::responseDelta,
        this,
        [this](const QString &delta) {
            writeStartupLog(QString("MainWindow: ai responseDelta len=%1")
                                .arg(QString::number(delta.size())));
            m_problemPage->appendAiResponse(delta);
        });
    connect(
        m_aiService,
        &AiService::thinkingChanged,
        this,
        [this](bool thinking) {
            m_problemPage->showAiThinking(thinking);
        });
    connect(
        m_aiService,
        &AiService::responseReady,
        this,
        [this](const QString &text) {
            writeStartupLog(QString("MainWindow: ai responseReady len=%1")
                                .arg(QString::number(text.size())));
            m_problemPage->showAiResponse(text);
        });
    connect(
        m_aiService,
        &AiService::failed,
        this,
        [this](const QString &message) {
            writeStartupLog(QString("MainWindow: ai failed %1")
                                .arg(message.left(200).replace('\n', ' ')));
            m_problemPage->showAiFailed(message);
            if (m_problemPage->isProblemTranslating()) {
                m_problemPage->showProblemTranslationFailed(message);
            }
        });
    connect(
        m_aiService,
        &AiService::problemTranslationReady,
        this,
        [this](const QString &description,
               const QString &inputSpec,
               const QString &outputSpec,
               const QString &hint) {
            if (m_hasCurrentProblem && !m_currentProblemInfo.problemUrl.isEmpty()) {
                ProblemTranslationInfo translationInfo;
                translationInfo.description = description;
                translationInfo.inputSpec = inputSpec;
                translationInfo.outputSpec = outputSpec;
                translationInfo.hint = hint;
                m_favoriteProblemRepository->saveProblemTranslation(
                    m_currentProblemInfo.problemUrl, translationInfo);
            }
            m_problemPage->applyProblemTranslation(description,
                                                   inputSpec,
                                                   outputSpec,
                                                   hint);
        });
    connect(
        m_problemPage,
        &ProblemPage::translateProblemRequested,
        this,
        [this]() {
            if (m_problemPage->hasCachedProblemTranslation()) {
                m_problemPage->showCachedProblemTranslation();
                return;
            }
            m_problemPage->showProblemTranslating(true);
            m_aiService->translateProblem(m_problemPage->currentProblemDescription(),
                                          m_problemPage->currentProblemInputSpec(),
                                          m_problemPage->currentProblemOutputSpec(),
                                          m_problemPage->currentProblemHint());
        });
    connect(
        m_client,
        &OpenJudgeClient::judgeFinished,
        this,
        [this](const NetworkResult &result) {
            m_problemPage->showTesting(false);
            const QString toolResultText = m_lastTestRequestLog + "\n\n" + formatJudgeResult(result);
            if (!result.ok) {
                const QString failureText =
                    m_lastTestRequestLog
                    + QString("\n\nJudge Response\nHTTP status: %1\nNetwork error: %2")
                          .arg(QString::number(result.statusCode), result.errorString);
                m_problemPage->showTestFailed(failureText);
                if (!m_pendingAiRunTestCallId.isEmpty()) {
                    m_aiService->failToolCall(m_pendingAiRunTestCallId, failureText);
                    m_pendingAiRunTestCallId.clear();
                }
                return;
            }
            m_problemPage->showTestResult(extractJudgeDisplayText(result));
            if (!m_pendingAiRunTestCallId.isEmpty()) {
                m_aiService->completeToolCall(m_pendingAiRunTestCallId, toolResultText);
                m_pendingAiRunTestCallId.clear();
            }
        });
    connect(
        m_favoriteProblemService,
        &FavoriteProblemService::favoriteLoaded,
        this,
        [this](const ProblemPageInfo &problemPageInfo) {
            setCurrentProblem(problemPageInfo);
            m_problemPage->showProblemLoadedFromFavorites(problemPageInfo);
            ProblemTranslationInfo translationInfo;
            if (m_favoriteProblemRepository->loadProblemTranslation(
                    problemPageInfo.problemUrl, &translationInfo)) {
                m_problemPage->applyCachedProblemTranslation(translationInfo.description,
                                                             translationInfo.inputSpec,
                                                             translationInfo.outputSpec,
                                                             translationInfo.hint);
            }
            if (!problemPageInfo.submitUrl.isEmpty()) {
                m_problemPage->openSubmit(problemPageInfo);
                m_submitService->openSubmit(QUrl(problemPageInfo.submitUrl));
            }
        });
    connect(
        m_favoriteProblemService,
        &FavoriteProblemService::failed,
        this,
        [this](const QString &message) {
            m_favoritePage->showFavoriteOperationFailed(message);
            if (ui->pageStack->currentWidget() == m_problemPage) {
                m_problemPage->showProblemLoadFailed(message);
            }
        });
    connect(
        m_resultService,
        &ResultService::loadingChanged,
        this,
        [this](bool loading) {
            Q_UNUSED(loading);
        });
    connect(
        m_resultService,
        &ResultService::resultLoaded,
        this,
        [this](const ResultPageInfo &resultPageInfo) {
            m_problemPage->appendResultPageInfo(resultPageInfo);
            writeStartupLog(QString("MainWindow: resultLoaded status=%1 submitId=%2 awaitingAi=%3")
                                .arg(resultPageInfo.statusText,
                                     resultPageInfo.submissionId,
                                     m_pendingAiSubmitAwaitingResult ? "true" : "false"));
            if (!m_pendingAiSubmitCallId.isEmpty() && m_pendingAiSubmitAwaitingResult) {
                const QString waitingStatus = resultPageInfo.statusText.trimmed().toLower();
                if (waitingStatus != "waiting") {
                    m_aiService->completeToolCall(
                        m_pendingAiSubmitCallId,
                        QString("Submit completed.\n%1")
                            .arg(formatResultPageSummary(resultPageInfo)));
                    m_pendingAiSubmitCallId.clear();
                    m_pendingAiSubmitAwaitingResult = false;
                }
            }
        });
    connect(
        m_resultService,
        &ResultService::failed,
        this,
        [this](const QString &message) {
            m_problemPage->appendResultFailure(message);
            writeStartupLog(QString("MainWindow: result load failed %1")
                                .arg(message.left(200).replace('\n', ' ')));
            if (!m_pendingAiSubmitCallId.isEmpty() && m_pendingAiSubmitAwaitingResult) {
                m_aiService->failToolCall(
                    m_pendingAiSubmitCallId,
                    QString("Result page load failed.\n%1").arg(message));
                m_pendingAiSubmitCallId.clear();
                m_pendingAiSubmitAwaitingResult = false;
            }
        });

    connect(
        m_submitService,
        &SubmitService::loadingChanged,
        this,
        [this](bool loading) {
            m_problemPage->showLoadingSubmitOptions(loading);
        });
    connect(
        m_submitService,
        &SubmitService::submittingChanged,
        this,
        [this](bool submitting) {
            m_problemPage->showSubmitting(submitting);
        });
    connect(
        m_submitService,
        &SubmitService::submitPageLoaded,
        this,
        [this](const SubmitPageInfo &submitPageInfo) {
            writeStartupLog("MainWindow: submitPageLoaded signal received");
            m_problemPage->showSubmitPageLoaded(
                submitPageInfo, m_submitService->defaultLanguage());
            writeStartupLog("MainWindow: problem page showSubmitPageLoaded returned");
        });
    connect(
        m_submitService,
        &SubmitService::submitPayloadBuilt,
        this,
        [this](const QString &languageValue,
               const QString &,
               const QByteArray &payload) {
            m_problemPage->showSubmitPayloadBuilt(languageValue, payload);
        });
    connect(
        m_submitService,
        &SubmitService::solutionSubmitted,
        this,
        [this](const NetworkResult &result) {
            QString redirectUrl;
            const QJsonDocument jsonDocument = QJsonDocument::fromJson(result.body);
            if (jsonDocument.isObject()) {
                redirectUrl = jsonDocument.object().value("redirect").toString();
            }
            writeStartupLog(QString("MainWindow: solutionSubmitted status=%1 redirect=%2 pendingAi=%3")
                                .arg(QString::number(result.statusCode),
                                     redirectUrl,
                                     m_pendingAiSubmitCallId.isEmpty() ? "false" : "true"));

            m_problemPage->showSubmitResult(result);
            if (!m_pendingAiSubmitCallId.isEmpty()) {
                if (!redirectUrl.isEmpty()) {
                    m_pendingAiSubmitAwaitingResult = true;
                } else {
                    m_aiService->completeToolCall(
                        m_pendingAiSubmitCallId,
                        formatSubmitResponse(result));
                    m_pendingAiSubmitCallId.clear();
                    m_pendingAiSubmitAwaitingResult = false;
                }
            }
            if (!redirectUrl.isEmpty()) {
                m_resultService->openResult(QUrl(redirectUrl));
            }
        });
    connect(
        m_submitService,
        &SubmitService::failed,
        this,
        [this](const QString &message) {
            m_problemPage->showSubmitFailed(message);
            if (!m_pendingAiSubmitCallId.isEmpty()) {
                m_aiService->failToolCall(
                    m_pendingAiSubmitCallId,
                    QString("Submit failed.\n%1").arg(message));
                m_pendingAiSubmitCallId.clear();
                m_pendingAiSubmitAwaitingResult = false;
            }
        });

    connect(
        m_homeService,
        &HomeService::loadingChanged,
        this,
        [this](bool loading) {
            if (loading) {
                m_homePage->showLoadingClasses();
            }
        });
    connect(
        m_homeService,
        &HomeService::failed,
        this,
        [this](const QString &message) {
            m_homePage->showHomeLoadFailed(message);
        });
    connect(
        m_homeService,
        &HomeService::homeLoaded,
        this,
        [this](const QList<JoinedClassInfo> &classes) {
            showRootPage(m_homePage);
            m_homePage->showClasses(classes);
            m_homePage->showReminderLoading();
            m_reminderService->refreshReminders(classes);
        });
    connect(
        m_reminderService,
        &ReminderService::remindersUpdated,
        this,
        [this](const QList<DeadlineReminder> &reminders) {
            m_currentReminders = reminders;
            m_homePage->showReminders(reminders);
        });
    connect(
        m_deadlineAlarmService,
        &DeadlineAlarmService::alarmTriggered,
        this,
        [this](const DeadlineReminder &reminder, int hoursBefore) {
            const QString text = QString("%1\n%2\nDeadline: %3\n%4 hour(s) left")
                                     .arg(reminder.courseName,
                                          reminder.contestTitle,
                                          reminder.deadlineText,
                                          QString::number(hoursBefore));
            showAlarmNotification("Contest deadline alarm", text);
            playAlarmSound();
        });

    connect(
        m_classService,
        &ClassService::loadingChanged,
        this,
        [this](bool loading) {
            if (loading) {
                m_classPage->showLoadingContests();
            }
        });
    connect(
        m_classPage,
        &ClassPage::refreshRequested,
        this,
        [this]() {
            const QUrl classUrl = m_classService->currentClassUrl();
            if (!classUrl.isValid() || classUrl.scheme().isEmpty()) {
                return;
            }
            m_classPage->showLoadingContests();
            m_classService->openClass(classUrl);
        });
    connect(
        m_classService,
        &ClassService::failed,
        this,
        [this](const QString &message) {
            m_classPage->showClassLoadFailed(message);
        });
    connect(
        m_classService,
        &ClassService::classLoaded,
        this,
        [this](const ClassPageInfo &, const GroupPageInfo &groupPageInfo) {
            m_classPage->showContestSets(groupPageInfo);
        });

    connect(
        m_contestService,
        &ContestService::loadingChanged,
        this,
        [this](bool loading) {
            if (loading) {
                m_contestPage->showLoadingProblems();
            }
        });
    connect(
        m_contestPage,
        &ContestPage::refreshRequested,
        this,
        [this]() {
            const QUrl contestUrl = m_contestService->currentContestUrl();
            if (!contestUrl.isValid() || contestUrl.scheme().isEmpty()) {
                return;
            }
            m_contestPage->showLoadingProblems();
            m_contestService->openContest(contestUrl);
        });
    connect(
        m_contestService,
        &ContestService::failed,
        this,
        [this](const QString &message) {
            m_contestPage->showContestLoadFailed(message);
        });
    connect(
        m_contestService,
        &ContestService::contestLoaded,
        this,
        [this](const ContestPageInfo &contestPageInfo) {
            m_contestPage->showProblems(contestPageInfo);
        });

    connect(
        m_problemService,
        &ProblemService::loadingChanged,
        this,
        [this](bool loading) {
            Q_UNUSED(loading);
        });
    connect(
        m_problemService,
        &ProblemService::failed,
        this,
        [this](const QString &message) {
            m_problemPage->showProblemLoadFailed(message);
        });
    connect(
        m_problemService,
        &ProblemService::problemLoaded,
        this,
        [this](const ProblemPageInfo &problemPageInfo) {
            writeStartupLog("MainWindow: problemLoaded signal received");
            setCurrentProblem(problemPageInfo);
            writeStartupLog("MainWindow: current problem set");
            m_problemPage->showProblem(problemPageInfo);
            ProblemTranslationInfo translationInfo;
            if (m_favoriteProblemRepository->loadProblemTranslation(
                    problemPageInfo.problemUrl, &translationInfo)) {
                m_problemPage->applyCachedProblemTranslation(translationInfo.description,
                                                             translationInfo.inputSpec,
                                                             translationInfo.outputSpec,
                                                             translationInfo.hint);
            }
            writeStartupLog("MainWindow: problem page content shown");
            if (!problemPageInfo.submitUrl.isEmpty()) {
                writeStartupLog("MainWindow: openSubmit begin");
                m_problemPage->openSubmit(problemPageInfo);
                writeStartupLog("MainWindow: problem page openSubmit returned");
                m_submitService->openSubmit(QUrl(problemPageInfo.submitUrl));
                writeStartupLog("MainWindow: submitService openSubmit called");
            }
        });
}

void MainWindow::showRootPage(QWidget *page)
{
    m_navigationHistory.clear();
    const int index = ui->pageStack->indexOf(page);
    if (index >= 0) {
        ui->pageStack->setCurrentIndex(index);
    }
}

void MainWindow::pushPage(QWidget *page)
{
    const int currentIndex = ui->pageStack->currentIndex();
    const int nextIndex = ui->pageStack->indexOf(page);
    if (nextIndex < 0 || currentIndex == nextIndex) {
        return;
    }

    m_navigationHistory.append(currentIndex);
    ui->pageStack->setCurrentIndex(nextIndex);
}

void MainWindow::popPage()
{
    if (m_navigationHistory.isEmpty()) {
        showRootPage(m_homePage);
        return;
    }

    const int index = m_navigationHistory.takeLast();
    ui->pageStack->setCurrentIndex(index);
}

void MainWindow::setCurrentProblem(const ProblemPageInfo &problemPageInfo)
{
    m_currentProblemInfo = problemPageInfo;
    m_hasCurrentProblem = !problemPageInfo.problemUrl.isEmpty();
    m_problemPage->setFavoriteEnabled(m_hasCurrentProblem);
    m_problemPage->setSubmitEnabled(
        m_hasCurrentProblem && !problemPageInfo.submitUrl.isEmpty());

    ProblemMeta meta;
    meta.problemUrl = problemPageInfo.problemUrl;
    meta.title = problemPageInfo.title;
    if (m_hasCurrentProblem) {
        m_problemMetaService->loadMeta(problemPageInfo.problemUrl, &meta);
    }
    m_problemPage->setProblemMeta(meta);
}

void MainWindow::applyDarkMode(bool dark)
{
    m_darkMode = dark;
    LightModeIconHelper::setDarkModeEnabled(dark);
    if (ui != nullptr) {
        const QString rootStyle = dark
            ? QString(
                  "#centralwidget, #pageStack {"
                  "  background: #000000;"
                  "}")
            : QString(
                  "#centralwidget, #pageStack {"
                  "  background: #ffffff;"
                  "}");
        ui->centralwidget->setStyleSheet(rootStyle);
        ui->pageStack->setStyleSheet(rootStyle);
    }
    if (m_loginPage != nullptr) {
        m_loginPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_loginPage);
    }
    if (m_homePage != nullptr) {
        m_homePage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_homePage);
    }
    if (m_classPage != nullptr) {
        m_classPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_classPage);
    }
    if (m_contestPage != nullptr) {
        m_contestPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_contestPage);
    }
    if (m_problemPage != nullptr) {
        m_problemPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_problemPage);
    }
    if (m_favoritePage != nullptr) {
        m_favoritePage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_favoritePage);
    }
    if (m_taskBoardPage != nullptr) {
        m_taskBoardPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_taskBoardPage);
    }
    if (m_statsPage != nullptr) {
        m_statsPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_statsPage);
    }
    if (m_reviewPage != nullptr) {
        m_reviewPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_reviewPage);
    }
    if (m_storagePage != nullptr) {
        m_storagePage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_storagePage);
    }
    if (m_settingsPage != nullptr) {
        m_settingsPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_settingsPage);
    }
    if (m_aiConfigPage != nullptr) {
        m_aiConfigPage->setDarkMode(dark);
        LightModeIconHelper::refreshIcons(m_aiConfigPage);
    }
}

void MainWindow::openProblemPage(const QString &problemUrl,
                                 const QString &problemTitle,
                                 bool preferFavoriteCache)
{
    m_problemPage->openProblem(problemTitle);
    pushPage(m_problemPage);

    if (preferFavoriteCache
        && m_favoriteProblemService->tryLoadFavorite(problemUrl)) {
        return;
    }

    m_problemService->openProblem(QUrl(problemUrl));
}

void MainWindow::saveCurrentProblemToFavorites()
{
    if (!m_hasCurrentProblem) {
        return;
    }

    const QList<FavoriteFolderInfo> folders = m_favoriteProblemService->loadFolders();
    if (folders.isEmpty()) {
        const QString newFolderName = QInputDialog::getText(
            this,
            "Create Favorite Folder",
            "No favorite folders yet. Create one:");
        if (newFolderName.trimmed().isEmpty()) {
            return;
        }

        if (!m_favoriteProblemService->createFolder(newFolderName.trimmed())) {
            m_problemPage->showProblemLoadFailed(m_favoriteProblemService->lastError());
            return;
        }
    }

    const QList<FavoriteFolderInfo> selectableFolders = m_favoriteProblemService->loadFolders();
    if (selectableFolders.isEmpty()) {
        m_problemPage->showProblemLoadFailed("No favorite folder available.");
        return;
    }

    QStringList folderNames;
    for (const FavoriteFolderInfo &folder : selectableFolders) {
        folderNames.append(folder.name);
    }

    bool ok = false;
    const QString selectedFolderName = QInputDialog::getItem(
        this,
        "Select Favorite Folder",
        "Save current problem to:",
        folderNames,
        0,
        false,
        &ok);
    if (!ok || selectedFolderName.isEmpty()) {
        return;
    }

    qint64 folderId = -1;
    for (const FavoriteFolderInfo &folder : selectableFolders) {
        if (folder.name == selectedFolderName) {
            folderId = folder.id;
            break;
        }
    }
    if (folderId < 0) {
        m_problemPage->showProblemLoadFailed("Selected favorite folder not found.");
        return;
    }

    if (!m_favoriteProblemService->saveFavoriteToFolder(m_currentProblemInfo, folderId)) {
        m_problemPage->showProblemLoadFailed(m_favoriteProblemService->lastError());
        return;
    }

    if (ui->pageStack->currentWidget() == m_favoritePage) {
        m_favoritePage->showFavorites(
            folderId,
            selectedFolderName,
            m_favoriteProblemService->loadFavoritesInFolder(folderId));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_allowClose || m_trayIcon == nullptr || !m_trayIcon->isVisible()) {
        clearStartupLog();
        QMainWindow::closeEvent(event);
        return;
    }

    hide();
    event->ignore();
    if (!m_trayNoticeShown) {
        m_trayIcon->showMessage("oj-client",
                                "Application is still running in the system tray.",
                                QSystemTrayIcon::Information,
                                3000);
        m_trayNoticeShown = true;
    }
}

void MainWindow::restoreFromTray()
{
    showNormal();
    raise();
    activateWindow();
}

bool MainWindow::requiresEmailVerification(const QString &email) const
{
    const QString normalizedEmail = email.trimmed();
    if (normalizedEmail.isEmpty()) {
        return false;
    }

    if (normalizedEmail == m_verifiedEmail) {
        return false;
    }

    CachedLoginInfo loginInfo;
    return !m_loginCacheService->tryLoadLoginByEmail(normalizedEmail, &loginInfo);
}

void MainWindow::showAlarmNotification(const QString &title, const QString &text)
{
    if (m_trayIcon == nullptr || !m_trayIcon->isVisible()) {
        return;
    }
    m_trayIcon->showMessage(title, text, QSystemTrayIcon::Warning, 6000);
}

void MainWindow::playAlarmSound()
{
    if (m_alarmPlayer == nullptr || m_alarmAudioOutput == nullptr) {
        return;
    }
    if (m_alarmRingPath.trimmed().isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(m_alarmRingPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        if (m_storagePage != nullptr) {
            m_storagePage->showOperationFailed("Ring file not found.");
        }
        return;
    }

    m_alarmPlayer->stop();
    m_alarmPlayer->setSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    m_alarmPlayer->play();
}

void MainWindow::scheduleNextAlarmCheck()
{
    if (m_alarmCheckTimer == nullptr) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const int nextHourValue = (now.time().hour() + 1) % 24;
    QDate nextDate = now.date();
    if (nextHourValue == 0) {
        nextDate = nextDate.addDays(1);
    }
    const QDateTime nextHour(nextDate, QTime(nextHourValue, 0, 0, 0));
    const qint64 delayMs = qMax<qint64>(1000, now.msecsTo(nextHour));
    m_alarmCheckTimer->start(static_cast<int>(delayMs));
}

void MainWindow::runHourlyAlarmCheck()
{
    if (!m_alarmEnabled || m_deadlineAlarmService == nullptr) {
        return;
    }

    m_deadlineAlarmService->processReminders(m_currentReminders);
}

void MainWindow::applyLoginCacheState(const QString &email)
{
    const QString normalizedEmail = email.trimmed();
    if (normalizedEmail != m_verifiedEmail) {
        m_verifiedEmail.clear();
    }

    CachedLoginInfo loginInfo;
    if (m_loginCacheService->tryLoadLoginByEmail(normalizedEmail, &loginInfo)) {
        m_loginPage->setPassword(loginInfo.password);
        m_loginPage->setVerificationRequired(false);
        return;
    }

    if (!normalizedEmail.isEmpty()) {
        m_loginPage->setVerificationRequired(true);
    } else {
        m_loginPage->setVerificationRequired(false);
    }
}
