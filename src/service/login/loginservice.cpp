#include "service/login/loginservice.h"

#include "network/openjudgeclient.h"
#include "parser/loginparser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace
{
QString extractLoginErrorMessage(const NetworkResult &result)
{
    const QJsonDocument jsonDocument = QJsonDocument::fromJson(result.body);
    if (jsonDocument.isObject()) {
        const QString message = jsonDocument.object().value("message").toString();
        if (!message.isEmpty()) {
            return message;
        }
    }

    if (!result.errorString.isEmpty()) {
        return result.errorString;
    }

    return QString::fromUtf8(result.body);
}

QUrl fallbackPersonalHomeUrl(const QByteArray &html, const QUrl &baseUrl)
{
    const QString text = QString::fromUtf8(html);
    const QRegularExpression regex("/user/\\d+/?");
    const QRegularExpressionMatch match = regex.match(text);
    if (!match.hasMatch()) {
        return QUrl();
    }

    return baseUrl.resolved(QUrl(match.captured(0)));
}
}

LoginService::LoginService(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    connect(
        m_client,
        &OpenJudgeClient::loginFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_logging || m_waitingForHomePage) {
                return;
            }

            if (!result.ok || !result.body.contains("\"result\":\"SUCCESS\"")) {
                m_logging = false;
                emit loggingChanged(false);
                emit failed(extractLoginErrorMessage(result));
                return;
            }

            m_waitingForHomePage = true;
            m_client->fetchHomePage();
        });

    connect(
        m_client,
        &OpenJudgeClient::homePageFinished,
        this,
        [this](const NetworkResult &result) {
            if (!m_logging || !m_waitingForHomePage) {
                return;
            }

            m_waitingForHomePage = false;
            m_logging = false;
            emit loggingChanged(false);

            if (!result.ok) {
                emit failed(result.errorString);
                return;
            }

            const QString personalHomeUrl =
                LoginParser::extractPersonalHomeUrl(result.body, result.finalUrl);
            QUrl parsedUrl(personalHomeUrl);
            if (personalHomeUrl.isEmpty() || !parsedUrl.isValid()
                || parsedUrl.scheme().isEmpty()) {
                parsedUrl = fallbackPersonalHomeUrl(result.body, result.finalUrl);
            }

            if (!parsedUrl.isValid() || parsedUrl.scheme().isEmpty()) {
                emit failed("Login succeeded, but personal home URL was not found.");
                return;
            }

            emit loginSucceeded(parsedUrl);
        });
}

void LoginService::login(const QString &email, const QString &password)
{
    if (m_client == nullptr || m_logging) {
        return;
    }

    m_logging = true;
    m_waitingForHomePage = false;
    emit loggingChanged(true);
    m_client->login(email, password);
}

bool LoginService::isLoggingIn() const
{
    return m_logging;
}
