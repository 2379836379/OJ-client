#pragma once

#include <QByteArray>
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class EmailVerifyService : public QObject
{
    Q_OBJECT

public:
    explicit EmailVerifyService(QObject *parent = nullptr);

    void setServerUrl(const QUrl &ServerUrl);
    void sendCode(const QString &email);
    void verifyCode(const QString &email, const QString &code);
    bool isSending() const;
    bool isVerifying() const;

signals:
    void sendingChanged(bool sending);
    void verifyingChanged(bool verifying);
    void codeSent(const QString &message);
    void verified(const QString &email);
    void failed(const QString &message);

private:
    QString parseErrorMessage(const QByteArray &body,
                              const QString &fallback) const;

    QNetworkAccessManager m_manager;
    QUrl m_ServerUrl;
    bool m_sending = false;
    bool m_verifying = false;
};
