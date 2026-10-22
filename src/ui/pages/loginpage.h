#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showIdle();
    void showLoggingIn();
    void showLoginFailed(const QString &message);
    void showVerificationMessage(const QString &message);
    void showSendingVerification(bool sending);
    void showVerifyingCode(bool verifying);
    void setCredentials(const QString &email, const QString &password);
    QString email() const;
    QString password() const;
    void setPassword(const QString &password);
    QString verificationCode() const;
    void setVerificationRequired(bool required);
    void clearVerificationCode();

signals:
    void loginRequested(const QString &email, const QString &password);
    void emailEdited(const QString &email);
    void verificationRequested(const QString &email);

private:
    QLabel *m_statusLabel = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QWidget *m_verifyPanel = nullptr;
    QLineEdit *m_verifyCodeEdit = nullptr;
    QPushButton *m_verifyButton = nullptr;
    QPushButton *m_loginButton = nullptr;
    bool m_darkMode = false;
};
