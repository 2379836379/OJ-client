#pragma once

#include "repository/meta/problemmetarepository.h"
#include "service/favorite/favoriteproblemservice.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QFrame;
class QVBoxLayout;
class QComboBox;

class StatsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatsPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);

    void showStats(const QHash<QString, int> &statusCounts,
                   const QHash<QString, int> &tagCounts,
                   int notesCount,
                   const QList<FavoriteFolderInfo> &folders,
                   const QList<ProblemMeta> &reviewProblems);

signals:
    void homeRequested();
    void themeToggleRequested(bool dark);
    void refreshRequested();
    void problemSelected(const QString &title, const QString &url);

private:
    void clearLayout(QVBoxLayout *layout);
    void addStatRow(QVBoxLayout *layout,
                    const QString &label,
                    int value,
                    int maxValue);
    void rebuildReviewTagOptions();
    void renderReview();

    QVBoxLayout *m_summaryLayout = nullptr;
    QListWidget *m_reviewList = nullptr;
    QLabel *m_reviewLabel = nullptr;
    QComboBox *m_reviewTagFilter = nullptr;
    QComboBox *m_reviewDifficultyFilter = nullptr;
    QList<ProblemMeta> m_reviewProblems;
    bool m_darkMode = false;
};
