#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QPlainTextEdit;
class QFrame;
class QWidget;

class AiConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit AiConfigPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void setConfigText(const QString &path, const QString &content);
    QString currentConfigText() const;
    void showSaveSucceeded(const QString &path);
    void showSaveFailed(const QString &message);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void saveRequested(const QString &content);

private:
    void setToolsExpanded(bool expanded);

    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QLabel *m_pathLabel = nullptr;
    QPlainTextEdit *m_configEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
