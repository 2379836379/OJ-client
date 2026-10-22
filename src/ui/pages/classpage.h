#pragma once

#include "parser/groupparser.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QFrame;
class QWidget;

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
    void setToolsExpanded(bool expanded);

    QLabel *m_titleLabel = nullptr;
    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QListWidget *m_contestListWidget = nullptr;
    QString m_className;
    QString m_classUrl;
    bool m_toolsExpanded = true;
    bool m_darkMode = false;
};
