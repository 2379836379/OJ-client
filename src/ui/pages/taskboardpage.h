#pragma once

#include "repository/meta/problemmetarepository.h"

#include <QListWidget>
#include <QList>
#include <QString>
#include <QWidget>

class QLabel;
class QListWidgetItem;
class QPushButton;
class QFrame;
class QComboBox;
class QDropEvent;

class StatusListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit StatusListWidget(const QString &status, QWidget *parent = nullptr);
    QString status() const;

signals:
    void itemDropped(const QString &url, const QString &targetStatus);

protected:
    void dropEvent(QDropEvent *event) override;

private:
    QString m_status;
};

class TaskBoardPage : public QWidget
{
    Q_OBJECT

public:
    explicit TaskBoardPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showTasks(const QList<ProblemMeta> &allMeta);

signals:
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void problemSelected(const QString &title, const QString &url);
    void statusChangeRequested(const QString &url, const QString &newStatus);

private:
    StatusListWidget *listForStatus(const QString &status) const;
    void showStatusMenu(QListWidget *list, const QPoint &pos);
    void renderTasks();
    void rebuildTagFilterOptions();

    StatusListWidget *m_todoList = nullptr;
    StatusListWidget *m_doingList = nullptr;
    StatusListWidget *m_doneList = nullptr;
    StatusListWidget *m_redoList = nullptr;
    QLabel *m_statusLabel = nullptr;
    QComboBox *m_tagFilterCombo = nullptr;
    QComboBox *m_difficultyFilterCombo = nullptr;
    QComboBox *m_sortCombo = nullptr;
    QList<ProblemMeta> m_allMeta;
    bool m_darkMode = false;
};
