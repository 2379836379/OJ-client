#pragma once

#include "repository/meta/problemmetarepository.h"

#include <QList>
#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QFrame;
class QPlainTextEdit;

class ReviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit ReviewPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showDue(const QList<ProblemMeta> &dueProblems);

signals:
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void problemSelected(const QString &title, const QString &url);
    void gradeRequested(const QString &url, int grade);

private:
    void showDetail(const ProblemMeta &meta);
    void clearDetail();
    void setGradeButtonsEnabled(bool enabled);

    QListWidget *m_dueList = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_detailTitle = nullptr;
    QLabel *m_detailMeta = nullptr;
    QPlainTextEdit *m_detailNote = nullptr;
    QPushButton *m_openButton = nullptr;
    QList<QPushButton *> m_gradeButtons;
    QList<ProblemMeta> m_dueProblems;
    QString m_currentUrl;
    QString m_currentTitle;
    bool m_darkMode = false;
};
