#include "loginservice.h"

#include "network/openjudgeclient.h"
#include "parser/Homepage.h"

#include <QJsonDocument>
#include <QJsonObject>

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

bool isLoginSuccessful(const QByteArray &body)
{
    const QJsonDocument document = QJsonDocument::fromJson(body);
    return document.isObject()
        && document.object().value("result").toString() == QStringLiteral("SUCCESS");
}
}

LoginService::LoginService(OpenJudgeClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    if (m_client == nullptr) {
        return;
    }

    connect(m_client, &OpenJudgeClient::loginFinished, this,
            [this](const NetworkResult &result) {
                if (!m_logging || m_waitingForHomePage) {
                    return;
                }

                if (!result.ok || !isLoginSuccessful(result.body)) {
                    m_logging = false;
                    emit loggingChanged(false);
                    emit failed(extractLoginErrorMessage(result));
                    return;
                }

                m_waitingForHomePage = true;
                m_client->fetchHomePage();
            });

    connect(m_client, &OpenJudgeClient::homePageFinished, this,
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
                    HomepageParser::parseHomepage(result.body);
                const QUrl parsedUrl(personalHomeUrl);
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
