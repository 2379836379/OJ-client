#pragma once

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

#include <functional>

class QNetworkReply;
class CookieStore;

struct NetworkResult
{
    bool ok = false;
    int statusCode = 0;
    QNetworkReply::NetworkError networkError = QNetworkReply::NoError;
    QString errorString;
    QUrl requestUrl;
    QUrl finalUrl;
    QByteArray body;
};

class OpenJudgeClient : public QObject
{
    Q_OBJECT

public:
    explicit OpenJudgeClient(QObject *parent = nullptr);

    void setBaseUrl(const QString &baseUrl);
    void setJudgerBaseUrl(const QString &judgerBaseUrl);
    void setCookieStore(CookieStore *cookieStore);
    void logout();

    void login(const QString &username, const QString &password);
    void fetchHomePage();
    void fetchUserHomePage(const QUrl &url);
    void fetchClassPage(const QUrl &url);
    void fetchGroupPage(const QUrl &url);
    void fetchContestPage(const QUrl &url);
    void fetchProblemPage(const QUrl &url);
    void fetchResultPage(const QUrl &url);
    void fetchSubmitPage(const QUrl &url);
    void submitSolution(const QUrl &submitActionUrl,
                        const QByteArray &payload,
                        const QUrl &refererUrl = QUrl());
    void judgeSource(const QString &language,
                     const QString &fileName,
                     const QByteArray &sourceCode,
                     const QByteArray &stdinText,
                     int timeLimitMs = 2000,
                     int memoryLimitMb = 256);
    void fetchProblemList(int page = 1);
    void fetchProblemDetail(const QString &problemId);
    void fetchSubmissionList(int page = 1);

signals:
    void loginFinished(const NetworkResult &result);
    void homePageFinished(const NetworkResult &result);
    void userHomePageFinished(const NetworkResult &result);
    void classPageFinished(const NetworkResult &result);
    void groupPageFinished(const NetworkResult &result);
    void contestPageFinished(const NetworkResult &result);
    void problemPageFinished(const NetworkResult &result);
    void resultPageFinished(const NetworkResult &result);
    void submitPageFinished(const NetworkResult &result);
    void solutionSubmitted(const NetworkResult &result);
    void judgeFinished(const NetworkResult &result);
    void problemListFinished(const NetworkResult &result);
    void problemDetailFinished(const NetworkResult &result);
    void submissionListFinished(const NetworkResult &result);

private:
    QNetworkRequest createRequest(const QUrl &url,
                                  const QUrl &refererUrl = QUrl()) const;
    void connectReply(
        QNetworkReply *reply,
        const std::function<void(const NetworkResult &)> &callback);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl = "http://openjudge.cn";
    QString m_judgerBaseUrl = "http://10.129.240.62:18080";
    CookieStore *m_cookieStore = nullptr;
};
