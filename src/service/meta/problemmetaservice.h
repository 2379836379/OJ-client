#pragma once

#include "repository/meta/problemmetarepository.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class ProblemMetaService : public QObject
{
    Q_OBJECT

public:
    explicit ProblemMetaService(ProblemMetaRepository *repository,
                                QObject *parent = nullptr);

    bool initialize();
    bool saveMeta(const ProblemMeta &meta);
    bool loadMeta(const QString &problemUrl, ProblemMeta *meta) const;
    QList<ProblemMeta> loadAllMeta() const;
    bool setTags(const QString &problemUrl, const QStringList &tagNames);
    QStringList loadAllTags() const;
    QHash<QString, int> statusCounts() const;
    QHash<QString, int> tagCounts() const;
    QList<ProblemMeta> reviewProblems() const;
    int notesCount() const;
    QList<ProblemMeta> dueReviewProblems() const;
    int dueReviewCount() const;
    bool gradeReview(const QString &problemUrl, int grade);
    QString lastError() const;

signals:
    void failed(const QString &message);
    void metaSaved(const ProblemMeta &meta);

private:
    ProblemMetaRepository *m_repository = nullptr;
};
