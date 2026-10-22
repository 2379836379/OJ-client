#include "service/login/emailverifyservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

EmailVerifyService::EmailVerifyService(QObject *parent)
    : QObject(parent)
{
}

void EmailVerifyService::setBaseUrl(const QUrl &baseUrl)
{
    if (baseUrl.isValid() && !baseUrl.scheme().isEmpty()) {
        m_baseUrl = baseUrl;
    }
}

void EmailVerifyService::sendCode(const QString &email)
{
    if (m_sending || m_verifying) {
        return;
    }

    QUrl url = m_baseUrl.resolved(QUrl("/auth/send-code"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json; charset=utf-8");

    const QJsonObject payload{
        {"email", email.trimmed()}
    };

    m_sending = true;
    emit sendingChanged(true);

    QNetworkReply *reply = m_manager.post(
        request,
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_sending = false;
        emit sendingChanged(false);

        const QByteArray body = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        if (!ok) {
            emit failed(parseErrorMessage(body, reply->errorString()));
            reply->deleteLater();
            return;
        }

        QString message = QStringLiteral("Verification code sent.");
        const QJsonDocument jsonDocument = QJsonDocument::fromJson(body);
        if (jsonDocument.isObject()) {
            const QString parsedMessage =
                jsonDocument.object().value("message").toString();
            if (!parsedMessage.isEmpty()) {
                message = parsedMessage;
            }
        }

        emit codeSent(message);
        reply->deleteLater();
    });
}

void EmailVerifyService::verifyCode(const QString &email, const QString &code)
{
    if (m_sending || m_verifying) {
        return;
    }

    QUrl url = m_baseUrl.resolved(QUrl("/auth/login-code"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json; charset=utf-8");

    const QJsonObject payload{
        {"email", email.trimmed()},
        {"code", code.trimmed()}
    };

    m_verifying = true;
    emit verifyingChanged(true);

    QNetworkReply *reply = m_manager.post(
        request,
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, email]() {
        m_verifying = false;
        emit verifyingChanged(false);

        const QByteArray body = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        if (!ok) {
            emit failed(parseErrorMessage(body, reply->errorString()));
            reply->deleteLater();
            return;
        }

        emit verified(email.trimmed());
        reply->deleteLater();
    });
}

bool EmailVerifyService::isSending() const
{
    return m_sending;
}

bool EmailVerifyService::isVerifying() const
{
    return m_verifying;
}

QString EmailVerifyService::parseErrorMessage(const QByteArray &body,
                                              const QString &fallback) const
{
    const QJsonDocument jsonDocument = QJsonDocument::fromJson(body);
    if (jsonDocument.isObject()) {
        const QJsonObject object = jsonDocument.object();
        const QString error = object.value("error").toString();
        if (!error.isEmpty()) {
            return error;
        }

        const QString message = object.value("message").toString();
        if (!message.isEmpty()) {
            return message;
        }
    }

    if (!fallback.isEmpty()) {
        return fallback;
    }

    const QString bodyText = QString::fromUtf8(body).trimmed();
    return bodyText.isEmpty() ? QStringLiteral("Verification request failed.")
                              : bodyText;
}
