#pragma once

#include "parser/GroupPage.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class PageShell;
class QPushButton;

class ClassPage : public QWidget
{
    Q_OBJECT

public:
    explicit ClassPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void openClass(const QString &name, const QString &url);
    void showLoadingContests();
    void showClassLoadFailed(const QString &message);
    void showContestSets(const GroupPageInfo &groupPageInfo);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void contestSelected(const QString &title, const QString &url);

private:
    PageShell *m_shell = nullptr;
    QLabel *m_titleLabel = nullptr;
    QListWidget *m_contestListWidget = nullptr;
    QPushButton *m_themeToolButton = nullptr;
    QPushButton *m_collapsedThemeToolButton = nullptr;
    QString m_className;
    QString m_classUrl;
    bool m_darkMode = false;
};
