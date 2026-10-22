#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QFrame;

// Editor for the service URLs stored in appstate.toml (OpenJudge / judger /
// email-verify). Alarm, ringtone and AI config have their own pages.
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void setUrls(const QString &openJudgeUrl,
                 const QString &judgerUrl,
                 const QString &emailVerifyUrl);
    void showSaveSucceeded();
    void showSaveFailed(const QString &message);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void saveRequested(const QString &openJudgeUrl,
                       const QString &judgerUrl,
                       const QString &emailVerifyUrl);

private:
    void setToolsExpanded(bool expanded);

    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QLineEdit *m_openJudgeEdit = nullptr;
    QLineEdit *m_judgerEdit = nullptr;
    QLineEdit *m_emailVerifyEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
