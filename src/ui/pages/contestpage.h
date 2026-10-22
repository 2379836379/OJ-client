#pragma once

#include "parser/contestparser.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QFrame;

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
    void setToolsExpanded(bool expanded);

    QLabel *m_titleLabel = nullptr;
    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QListWidget *m_problemListWidget = nullptr;
    QString m_contestTitle;
    QString m_contestUrl;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
