#include "service/meta/problemmetaservice.h"

#include <QDateTime>

ProblemMetaService::ProblemMetaService(ProblemMetaRepository *repository,
                                       QObject *parent)
    : QObject(parent)
    , m_repository(repository)
{
}

bool ProblemMetaService::initialize()
{
    if (m_repository == nullptr) {
        emit failed("Problem meta repository not available");
        return false;
    }

    if (!m_repository->initialize()) {
        emit failed(m_repository->lastError());
        return false;
    }
    return true;
}

bool ProblemMetaService::saveMeta(const ProblemMeta &meta)
{
    if (m_repository == nullptr) {
        emit failed("Problem meta repository not available");
        return false;
    }

    if (!m_repository->upsertMeta(meta)) {
        emit failed(m_repository->lastError());
        return false;
    }
    if (!m_repository->setTags(meta.problemUrl, meta.tags)) {
        emit failed(m_repository->lastError());
        return false;
    }

    if (meta.reviewFlag) {
        m_repository->ensureReviewScheduled(
            meta.problemUrl, QDateTime::currentDateTime().toString(Qt::ISODate));
    }

    emit metaSaved(meta);
    return true;
}

bool ProblemMetaService::loadMeta(const QString &problemUrl, ProblemMeta *meta) const
{
    if (m_repository == nullptr) {
        return false;
    }
    return m_repository->loadMeta(problemUrl, meta);
}

QList<ProblemMeta> ProblemMetaService::loadAllMeta() const
{
    return m_repository == nullptr ? QList<ProblemMeta>()
                                   : m_repository->loadAllMeta();
}

bool ProblemMetaService::setTags(const QString &problemUrl, const QStringList &tagNames)
{
    if (m_repository == nullptr) {
        emit failed("Problem meta repository not available");
        return false;
    }
    if (!m_repository->setTags(problemUrl, tagNames)) {
        emit failed(m_repository->lastError());
        return false;
    }
    return true;
}

QStringList ProblemMetaService::loadAllTags() const
{
    return m_repository == nullptr ? QStringList() : m_repository->loadAllTags();
}

QHash<QString, int> ProblemMetaService::statusCounts() const
{
    return m_repository == nullptr ? QHash<QString, int>()
                                   : m_repository->statusCounts();
}

QHash<QString, int> ProblemMetaService::tagCounts() const
{
    return m_repository == nullptr ? QHash<QString, int>()
                                   : m_repository->tagCounts();
}

QList<ProblemMeta> ProblemMetaService::reviewProblems() const
{
    return m_repository == nullptr ? QList<ProblemMeta>()
                                   : m_repository->reviewProblems();
}

int ProblemMetaService::notesCount() const
{
    return m_repository == nullptr ? 0 : m_repository->notesCount();
}

QList<ProblemMeta> ProblemMetaService::dueReviewProblems() const
{
    if (m_repository == nullptr) {
        return QList<ProblemMeta>();
    }
    return m_repository->dueReviewProblems(
        QDateTime::currentDateTime().toString(Qt::ISODate));
}

int ProblemMetaService::dueReviewCount() const
{
    if (m_repository == nullptr) {
        return 0;
    }
    return m_repository->dueReviewCount(
        QDateTime::currentDateTime().toString(Qt::ISODate));
}

bool ProblemMetaService::gradeReview(const QString &problemUrl, int grade)
{
    if (m_repository == nullptr) {
        emit failed("Problem meta repository not available");
        return false;
    }

    ProblemMeta meta;
    if (!m_repository->loadMeta(problemUrl, &meta)) {
        emit failed("Problem not found for review grading");
        return false;
    }

    // Simplified SM-2: grade 0=Again, 1=Hard, 2=Good, 3=Easy.
    int interval = meta.reviewInterval;
    int ease = meta.reviewEase;

    if (grade <= 0) {
        interval = 0;
        ease = qMax(130, ease - 20);
    } else if (grade == 1) {
        ease = qMax(130, ease - 15);
        interval = interval <= 0 ? 1 : qMax(1, (interval * 120 + 50) / 100);
    } else if (grade == 2) {
        interval = interval <= 0 ? 1 : qMax(1, (interval * ease + 50) / 100);
    } else {
        ease = ease + 15;
        interval = interval <= 0 ? 2 : qMax(1, (interval * ease * 13 + 500) / 1000);
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QString nextReviewAt = now.addDays(interval).toString(Qt::ISODate);

    if (!m_repository->updateReviewSchedule(problemUrl,
                                            interval,
                                            ease,
                                            nextReviewAt,
                                            now.toString(Qt::ISODate),
                                            meta.reviewCount + 1)) {
        emit failed(m_repository->lastError());
        return false;
    }
    return true;
}

QString ProblemMetaService::lastError() const
{
    return m_repository == nullptr ? QString() : m_repository->lastError();
}
