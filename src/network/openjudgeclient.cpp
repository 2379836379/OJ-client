#include "openjudgeclient.h"
#include "cookiestore.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QTextStream>

namespace
{
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

QString normalizeBaseUrl(const QString &baseUrl)
{
    QString result = baseUrl.trimmed();
    while (result.endsWith('/')) {
        result.chop(1);
    }
    return result;
}
}

OpenJudgeClient::OpenJudgeClient(QObject *parent)
    : QObject(parent)
{
}
void OpenJudgeClient::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = normalizeBaseUrl(baseUrl);
}

void OpenJudgeClient::setJudgerBaseUrl(const QString &judgerBaseUrl)
{
    const QString normalized = normalizeBaseUrl(judgerBaseUrl);
    if (!normalized.isEmpty()) {
        m_judgerBaseUrl = normalized;
    }
}

void OpenJudgeClient::setCookieStore(CookieStore *cookieStore)
{
    m_cookieStore = cookieStore;
    if (m_cookieStore) {
        m_manager.setCookieJar(m_cookieStore);
        if (!m_cookieStore->parent()) {
            m_cookieStore->setParent(this);
        }
    }
}

void OpenJudgeClient::logout()
{
    if (m_cookieStore != nullptr) {
        m_cookieStore->clearCookies();
    }
}

void OpenJudgeClient::login(const QString &username, const QString &password)
{
    QUrl url(m_baseUrl + "/api/auth/login/");
    QNetworkRequest request = createRequest(url);

    QUrlQuery form;
    form.addQueryItem("email", username);
    form.addQueryItem("password", password);
    form.addQueryItem("redirectUrl", m_baseUrl);

    QNetworkReply *reply = m_manager.post(
        request,
        form.toString(QUrl::FullyEncoded).toUtf8());

    connectReply(reply, [this](const NetworkResult &result) {
        emit loginFinished(result);
    });
}

void OpenJudgeClient::fetchHomePage()
{
    const QUrl url(m_baseUrl);
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit homePageFinished(result);
    });
}

void OpenJudgeClient::fetchUserHomePage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit userHomePageFinished(result);
    });
}

void OpenJudgeClient::fetchClassPage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit classPageFinished(result);
    });
}

void OpenJudgeClient::fetchGroupPage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit groupPageFinished(result);
    });
}

void OpenJudgeClient::fetchContestPage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit contestPageFinished(result);
    });
}

void OpenJudgeClient::fetchProblemPage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit problemPageFinished(result);
    });
}

void OpenJudgeClient::fetchResultPage(const QUrl &url)
{
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit resultPageFinished(result);
    });
}

void OpenJudgeClient::fetchSubmitPage(const QUrl &url)
{
    writeStartupLog("OpenJudgeClient::fetchSubmitPage begin");
    QNetworkReply *reply = m_manager.get(createRequest(url));
    writeStartupLog("OpenJudgeClient::fetchSubmitPage request issued");
    connectReply(reply, [this](const NetworkResult &result) {
        writeStartupLog("OpenJudgeClient::fetchSubmitPage reply finished");
        emit submitPageFinished(result);
    });
    writeStartupLog("OpenJudgeClient::fetchSubmitPage connectReply attached");
}

void OpenJudgeClient::submitSolution(const QUrl &submitActionUrl,
                                     const QByteArray &payload,
                                     const QUrl &refererUrl)
{
    QNetworkRequest request = createRequest(submitActionUrl, refererUrl);
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    QNetworkReply *reply = m_manager.post(request, payload);
    connectReply(reply, [this](const NetworkResult &result) {
        emit solutionSubmitted(result);
    });
}

void OpenJudgeClient::judgeSource(const QString &language,
                                  const QString &fileName,
                                  const QByteArray &sourceCode,
                                  const QByteArray &stdinText,
                                  int timeLimitMs,
                                  int memoryLimitMb)
{
    QUrl url(m_judgerBaseUrl + "/judge");
    QNetworkRequest request(url);
    request.setTransferTimeout(15000);
    request.setHeader(
        QNetworkRequest::UserAgentHeader,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json");

    auto *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto appendTextPart = [multipart](const QByteArray &name, const QByteArray &value) {
        QHttpPart part;
        part.setHeader(
            QNetworkRequest::ContentDispositionHeader,
            QVariant(QString("form-data; name=\"%1\"").arg(QString::fromUtf8(name))));
        part.setBody(value);
        multipart->append(part);
    };

    appendTextPart("language", language.toUtf8());
    appendTextPart("stdin", stdinText);
    appendTextPart("time_limit_ms", QByteArray::number(timeLimitMs));
    appendTextPart("memory_limit_mb", QByteArray::number(memoryLimitMb));

    QHttpPart filePart;
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    filePart.setBody(sourceCode);
    multipart->append(filePart);

    QNetworkReply *reply = m_manager.post(request, multipart);
    multipart->setParent(reply);
    connectReply(reply, [this](const NetworkResult &result) {
        emit judgeFinished(result);
    });
}

void OpenJudgeClient::fetchProblemList(int page)
{
    QUrl url(m_baseUrl + "/problem");
    QUrlQuery query;
    query.addQueryItem("page", QString::number(page));
    url.setQuery(query);

    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit problemListFinished(result);
    });
}

void OpenJudgeClient::fetchProblemDetail(const QString &problemId)
{
    const QUrl url(m_baseUrl + "/problem/" + problemId);
    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit problemDetailFinished(result);
    });
}

void OpenJudgeClient::fetchSubmissionList(int page)
{
    QUrl url(m_baseUrl + "/status");
    QUrlQuery query;
    query.addQueryItem("page", QString::number(page));
    url.setQuery(query);

    QNetworkReply *reply = m_manager.get(createRequest(url));
    connectReply(reply, [this](const NetworkResult &result) {
        emit submissionListFinished(result);
    });
}

QNetworkRequest OpenJudgeClient::createRequest(const QUrl &url,
                                               const QUrl &refererUrl) const
{
    QNetworkRequest request(url);

    request.setHeader(
        QNetworkRequest::UserAgentHeader,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36");
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    request.setRawHeader(
        "Accept",
        "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader(
        "Referer",
        (refererUrl.isEmpty() ? m_baseUrl : refererUrl.toString()).toUtf8());
    request.setRawHeader("Connection", "keep-alive");

    return request;
}

void OpenJudgeClient::connectReply(
    QNetworkReply *reply,
    const std::function<void(const NetworkResult &)> &callback)
{
    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        NetworkResult result;
        result.statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.networkError = reply->error();
        result.errorString = reply->errorString();
        result.requestUrl = reply->request().url();
        result.finalUrl = reply->url();
        result.body = reply->readAll();
        result.ok = (result.networkError == QNetworkReply::NoError);

        if (result.ok) {
            qDebug() << "network ok, status =" << result.statusCode
                     << "url =" << result.finalUrl;
        } else {
            qDebug() << "network failed:" << result.errorString
                     << "status =" << result.statusCode
                     << "url =" << result.requestUrl;
        }

        callback(result);
        reply->deleteLater();
    });
}
