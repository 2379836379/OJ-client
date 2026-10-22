#pragma once

#include <QObject>
#include <QUrl>

class OpenJudgeClient;

class LoginService : public QObject
{
    Q_OBJECT

public:
    explicit LoginService(OpenJudgeClient *client, QObject *parent = nullptr);

    void login(const QString &email, const QString &password);
    bool isLoggingIn() const;

signals:
    void loggingChanged(bool logging);
    void loginSucceeded(const QUrl &personalHomeUrl);
    void failed(const QString &message);

private:
    OpenJudgeClient *m_client = nullptr;
    bool m_logging = false;
    bool m_waitingForHomePage = false;
};
