#include "ui/pages/loginpage.h"
#include "ui/uitheme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString buildLoginPageStyle(bool dark)
{
    const UiTheme::Palette colors = UiTheme::palette(dark);
    return QStringLiteral(
               "LoginPage { background: %1; }"
               "#loginCard { background: %1; border: 1px solid %2; border-radius: 18px; }"
               "#loginTitleLabel { font-size: 28px; font-weight: 600; color: %3; margin-bottom: 8px; }"
               "QLabel { color: %3; }"
               "QLineEdit {"
               "  padding: 10px 12px;"
               "  border: 1px solid %2;"
               "  border-radius: 10px;"
               "  background: %1;"
               "  color: %3;"
               "  selection-background-color: %4;"
               "}"
               "#loginButton, #verifyButton {"
               "  padding: 10px 14px;"
               "  border: 1px solid %2;"
               "  border-radius: 10px;"
               "  background: %4;"
               "  color: %3;"
               "  font-weight: 600;"
               "}"
               "#verifyButton { padding: 10px 12px; background: %1; }"
               "#loginButton:hover, #verifyButton:hover { background: %5; }"
               "#loginStatusLabel { color: %3; min-height: 20px; }")
        .arg(colors.page, colors.border, colors.text, colors.selected, colors.hover);
}
}

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *shell = new QFrame(this);
    shell->setObjectName("loginShell");
    auto *shellLayout = new QVBoxLayout(shell);
    shellLayout->setContentsMargins(0, 0, 0, 0);
    shellLayout->addStretch();

    auto *rowLayout = new QHBoxLayout();
    rowLayout->addStretch();

    auto *card = new QFrame(shell);
    card->setObjectName("loginCard");
    card->setFixedWidth(420);
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(14);

    auto *titleLabel = new QLabel("OpenJudge Login", card);
    titleLabel->setObjectName("loginTitleLabel");

    auto *emailLabel = new QLabel("Email", card);
    auto *passwordLabel = new QLabel("Password", card);
    auto *verifyLabel = new QLabel("Verification Code", card);

    m_emailEdit = new QLineEdit(card);
    m_emailEdit->setPlaceholderText("name@example.com");
    m_passwordEdit = new QLineEdit(card);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Password");
    m_verifyPanel = new QWidget(card);
    auto *verifyLayout = new QHBoxLayout(m_verifyPanel);
    verifyLayout->setContentsMargins(0, 0, 0, 0);
    verifyLayout->setSpacing(10);
    m_verifyCodeEdit = new QLineEdit(m_verifyPanel);
    m_verifyCodeEdit->setPlaceholderText("Email code");
    m_verifyButton = new QPushButton("Verify", m_verifyPanel);
    m_verifyButton->setObjectName("verifyButton");
    verifyLayout->addWidget(m_verifyCodeEdit, 1);
    verifyLayout->addWidget(m_verifyButton);

    m_statusLabel = new QLabel(card);
    m_statusLabel->setObjectName("loginStatusLabel");
    m_statusLabel->setWordWrap(true);

    m_loginButton = new QPushButton("Login", card);
    m_loginButton->setObjectName("loginButton");

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(emailLabel);
    cardLayout->addWidget(m_emailEdit);
    cardLayout->addWidget(passwordLabel);
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addWidget(verifyLabel);
    cardLayout->addWidget(m_verifyPanel);
    cardLayout->addWidget(m_statusLabel);
    cardLayout->addWidget(m_loginButton);

    rowLayout->addWidget(card);
    rowLayout->addStretch();
    shellLayout->addLayout(rowLayout);
    shellLayout->addStretch();

    layout->addWidget(shell);

    setDarkMode(false);

    connect(
        m_loginButton,
        &QPushButton::clicked,
        this,
        [this]() {
            emit loginRequested(
                m_emailEdit->text().trimmed(),
                m_passwordEdit->text());
        });
    connect(
        m_passwordEdit,
        &QLineEdit::returnPressed,
        this,
        [this]() {
            emit loginRequested(
                m_emailEdit->text().trimmed(),
                m_passwordEdit->text());
        });
    connect(
        m_verifyButton,
        &QPushButton::clicked,
        this,
        [this]() {
            emit verificationRequested(m_emailEdit->text().trimmed());
        });
    connect(
        m_emailEdit,
        &QLineEdit::textEdited,
        this,
        [this](const QString &text) {
            emit emailEdited(text.trimmed());
        });

    showIdle();
    setVerificationRequired(false);
}

void LoginPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    setStyleSheet(buildLoginPageStyle(dark));
}

void LoginPage::showIdle()
{
    m_emailEdit->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_verifyCodeEdit->setEnabled(true);
    m_verifyButton->setEnabled(m_verifyPanel->isVisible());
    m_loginButton->setEnabled(true);
    m_statusLabel->clear();
}

void LoginPage::showLoggingIn()
{
    m_emailEdit->setEnabled(false);
    m_passwordEdit->setEnabled(false);
    m_verifyCodeEdit->setEnabled(false);
    m_verifyButton->setEnabled(false);
    m_loginButton->setEnabled(false);
    m_statusLabel->setText("Logging in...");
}

void LoginPage::showLoginFailed(const QString &message)
{
    m_emailEdit->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_verifyCodeEdit->setEnabled(true);
    m_verifyButton->setEnabled(m_verifyPanel->isVisible());
    m_loginButton->setEnabled(true);
    m_statusLabel->setText(message);
}

void LoginPage::showVerificationMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void LoginPage::showSendingVerification(bool sending)
{
    m_emailEdit->setEnabled(!sending);
    m_passwordEdit->setEnabled(!sending);
    m_verifyCodeEdit->setEnabled(!sending);
    m_verifyButton->setEnabled(!sending && m_verifyPanel->isVisible());
    m_loginButton->setEnabled(!sending);
    if (sending) {
        m_statusLabel->setText("Sending verification email...");
    }
}

void LoginPage::showVerifyingCode(bool verifying)
{
    m_emailEdit->setEnabled(!verifying);
    m_passwordEdit->setEnabled(!verifying);
    m_verifyCodeEdit->setEnabled(!verifying);
    m_verifyButton->setEnabled(!verifying && m_verifyPanel->isVisible());
    m_loginButton->setEnabled(!verifying);
    if (verifying) {
        m_statusLabel->setText("Verifying code...");
    }
}

void LoginPage::setCredentials(const QString &email, const QString &password)
{
    m_emailEdit->setText(email);
    m_passwordEdit->setText(password);
}

QString LoginPage::email() const
{
    return m_emailEdit->text().trimmed();
}

QString LoginPage::password() const
{
    return m_passwordEdit->text();
}

void LoginPage::setPassword(const QString &password)
{
    m_passwordEdit->setText(password);
}

QString LoginPage::verificationCode() const
{
    return m_verifyCodeEdit->text().trimmed();
}

void LoginPage::setVerificationRequired(bool required)
{
    m_verifyPanel->setVisible(required);
    QList<QLabel *> labels = findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text() == "Verification Code") {
            label->setVisible(required);
            break;
        }
    }
    if (!required) {
        m_verifyCodeEdit->clear();
    }
    m_verifyButton->setEnabled(required);
}

void LoginPage::clearVerificationCode()
{
    m_verifyCodeEdit->clear();
}
