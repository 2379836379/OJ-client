#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QFrame;

// Editor for the judger URL stored in appconfig.toml.
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void setServerUrl(const QString &serverUrl);
    void showSaveSucceeded();
    void showSaveFailed(const QString &message);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void saveRequested(const QString &serverUrl);

private:
    void setToolsExpanded(bool expanded);

    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QLineEdit *m_serverUrlEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
