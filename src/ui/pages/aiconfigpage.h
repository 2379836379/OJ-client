#pragma once

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class PageShell;

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
    PageShell *m_shell = nullptr;
    QLabel *m_pathLabel = nullptr;
    QPlainTextEdit *m_configEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
