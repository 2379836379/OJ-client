#pragma once

#include "parser/ContestPage.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class PageShell;

class ContestPage : public QWidget
{
    Q_OBJECT

public:
    explicit ContestPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void openContest(const QString &title, const QString &url);
    void showLoadingProblems();
    void showContestLoadFailed(const QString &message);
    void showProblems(const ContestPageInfo &contestPageInfo);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void problemSelected(const QString &title, const QString &url);

private:
    PageShell *m_shell = nullptr;
    QLabel *m_titleLabel = nullptr;
    QListWidget *m_problemListWidget = nullptr;
    QString m_contestTitle;
    QString m_contestUrl;
    bool m_darkMode = false;
};
