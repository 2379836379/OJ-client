#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

struct ProblemMeta
{
    QString problemUrl;
    QString title;
    QString note;
    int difficulty = 0;
    QString taskStatus = "todo";
    int priority = 0;
    QString deadline;
    bool reviewFlag = false;
    QStringList tags;
    int reviewInterval = 0;   // current spaced-repetition interval in days
    int reviewEase = 250;     // ease factor x100 (SM-2 style)
    QString nextReviewAt;     // ISO local datetime, empty = not scheduled
    QString lastReviewedAt;   // ISO local datetime of last grading
    int reviewCount = 0;      // number of completed reviews
};

class QSqlQuery;

class ProblemMetaRepository
{
public:
    ProblemMetaRepository();
    ~ProblemMetaRepository();

    bool initialize();
    bool upsertMeta(const ProblemMeta &meta);
    bool loadMeta(const QString &problemUrl, ProblemMeta *meta) const;
    QList<ProblemMeta> loadAllMeta() const;
    bool setTags(const QString &problemUrl, const QStringList &tagNames);
    QStringList loadTags(const QString &problemUrl) const;
    QStringList loadAllTags() const;
    QHash<QString, int> statusCounts() const;
    QHash<QString, int> tagCounts() const;
    QList<ProblemMeta> reviewProblems() const;
    int notesCount() const;
    QList<ProblemMeta> dueReviewProblems(const QString &nowIso) const;
    int dueReviewCount(const QString &nowIso) const;
    bool updateReviewSchedule(const QString &problemUrl,
                              int interval,
                              int ease,
                              const QString &nextReviewAt,
                              const QString &lastReviewedAt,
                              int reviewCount);
    bool ensureReviewScheduled(const QString &problemUrl, const QString &nowIso);
    QString lastError() const;

private:
    bool openDatabase();
    bool ensureSchema();
    void ensureReviewColumns();
    ProblemMeta readMeta(const QSqlQuery &query) const;

    QString m_connectionName;
    mutable QString m_lastError;
};
