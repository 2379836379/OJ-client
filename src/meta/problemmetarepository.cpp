#include "problemmetarepository.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>
#include <QVariant>

namespace
{
QString databasePath()
{
    return QDir(AppPaths::dataDir()).filePath("problemmeta.db");
}
}

ProblemMetaRepository::ProblemMetaRepository()
    : m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

ProblemMetaRepository::~ProblemMetaRepository()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName);
        database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool ProblemMetaRepository::initialize()
{
    if (!openDatabase()) {
        return false;
    }
    return ensureSchema();
}

bool ProblemMetaRepository::ensureSchema()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);

    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS problem_meta ("
        "problem_url TEXT PRIMARY KEY,"
        "title TEXT,"
        "note TEXT,"
        "difficulty INTEGER DEFAULT 0,"
        "task_status TEXT DEFAULT 'todo',"
        "priority INTEGER DEFAULT 0,"
        "deadline TEXT,"
        "review_flag INTEGER DEFAULT 0,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS tags ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS problem_tags ("
        "problem_url TEXT NOT NULL,"
        "tag_id INTEGER NOT NULL,"
        "PRIMARY KEY(problem_url, tag_id)"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    ensureReviewColumns();
    return true;
}

void ProblemMetaRepository::ensureReviewColumns()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);

    QStringList existing;
    QSqlQuery info(database);
    if (info.exec("PRAGMA table_info(problem_meta)")) {
        while (info.next()) {
            existing << info.value(1).toString();
        }
    }

    const QList<QPair<QString, QString>> columns = {
        {"review_interval", "INTEGER DEFAULT 0"},
        {"review_ease", "INTEGER DEFAULT 250"},
        {"next_review_at", "TEXT"},
        {"last_reviewed_at", "TEXT"},
        {"review_count", "INTEGER DEFAULT 0"}};

    for (const auto &column : columns) {
        if (existing.contains(column.first)) {
            continue;
        }
        QSqlQuery alter(database);
        alter.exec(QString("ALTER TABLE problem_meta ADD COLUMN %1 %2")
                       .arg(column.first, column.second));
    }
}

bool ProblemMetaRepository::upsertMeta(const ProblemMeta &meta)
{
    if (!openDatabase()) {
        return false;
    }

    const QString normalizedUrl = meta.problemUrl.trimmed();
    if (normalizedUrl.isEmpty()) {
        m_lastError = "Problem URL cannot be empty";
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO problem_meta "
        "(problem_url, title, note, difficulty, task_status, priority, deadline, review_flag, updated_at) "
        "VALUES (:problem_url, :title, :note, :difficulty, :task_status, :priority, :deadline, :review_flag, CURRENT_TIMESTAMP) "
        "ON CONFLICT(problem_url) DO UPDATE SET "
        "title = excluded.title,"
        "note = excluded.note,"
        "difficulty = excluded.difficulty,"
        "task_status = excluded.task_status,"
        "priority = excluded.priority,"
        "deadline = excluded.deadline,"
        "review_flag = excluded.review_flag,"
        "updated_at = CURRENT_TIMESTAMP");
    query.bindValue(":problem_url", normalizedUrl);
    query.bindValue(":title", meta.title);
    query.bindValue(":note", meta.note);
    query.bindValue(":difficulty", meta.difficulty);
    query.bindValue(":task_status", meta.taskStatus);
    query.bindValue(":priority", meta.priority);
    query.bindValue(":deadline", meta.deadline);
    query.bindValue(":review_flag", meta.reviewFlag ? 1 : 0);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProblemMetaRepository::loadMeta(const QString &problemUrl, ProblemMeta *meta) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT problem_url, title, note, difficulty, task_status, priority, deadline, review_flag, "
        "review_interval, review_ease, next_review_at, last_reviewed_at, review_count "
        "FROM problem_meta WHERE problem_url = :problem_url");
    query.bindValue(":problem_url", problemUrl.trimmed());
    if (!query.exec() || !query.next()) {
        return false;
    }

    if (meta != nullptr) {
        *meta = readMeta(query);
        meta->tags = loadTags(problemUrl);
    }
    return true;
}

QList<ProblemMeta> ProblemMetaRepository::loadAllMeta() const
{
    QList<ProblemMeta> result;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return result;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT problem_url, title, note, difficulty, task_status, priority, deadline, review_flag, "
            "review_interval, review_ease, next_review_at, last_reviewed_at, review_count "
            "FROM problem_meta ORDER BY updated_at DESC")) {
        return result;
    }

    while (query.next()) {
        ProblemMeta meta = readMeta(query);
        meta.tags = loadTags(meta.problemUrl);
        result << meta;
    }
    return result;
}

bool ProblemMetaRepository::setTags(const QString &problemUrl, const QStringList &tagNames)
{
    if (!openDatabase()) {
        return false;
    }

    const QString normalizedUrl = problemUrl.trimmed();
    if (normalizedUrl.isEmpty()) {
        m_lastError = "Problem URL cannot be empty";
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);

    QSqlQuery deleteQuery(database);
    deleteQuery.prepare("DELETE FROM problem_tags WHERE problem_url = :problem_url");
    deleteQuery.bindValue(":problem_url", normalizedUrl);
    if (!deleteQuery.exec()) {
        m_lastError = deleteQuery.lastError().text();
        return false;
    }

    QStringList seen;
    for (const QString &rawName : tagNames) {
        const QString name = rawName.trimmed();
        if (name.isEmpty() || seen.contains(name, Qt::CaseInsensitive)) {
            continue;
        }
        seen << name;

        QSqlQuery insertTag(database);
        insertTag.prepare("INSERT OR IGNORE INTO tags (name) VALUES (:name)");
        insertTag.bindValue(":name", name);
        if (!insertTag.exec()) {
            m_lastError = insertTag.lastError().text();
            return false;
        }

        QSqlQuery idQuery(database);
        idQuery.prepare("SELECT id FROM tags WHERE name = :name");
        idQuery.bindValue(":name", name);
        if (!idQuery.exec() || !idQuery.next()) {
            m_lastError = idQuery.lastError().text();
            return false;
        }
        const qint64 tagId = idQuery.value(0).toLongLong();

        QSqlQuery linkQuery(database);
        linkQuery.prepare(
            "INSERT OR IGNORE INTO problem_tags (problem_url, tag_id) "
            "VALUES (:problem_url, :tag_id)");
        linkQuery.bindValue(":problem_url", normalizedUrl);
        linkQuery.bindValue(":tag_id", tagId);
        if (!linkQuery.exec()) {
            m_lastError = linkQuery.lastError().text();
            return false;
        }
    }
    return true;
}

QStringList ProblemMetaRepository::loadTags(const QString &problemUrl) const
{
    QStringList tags;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return tags;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT t.name FROM problem_tags pt "
        "JOIN tags t ON t.id = pt.tag_id "
        "WHERE pt.problem_url = :problem_url "
        "ORDER BY t.name ASC");
    query.bindValue(":problem_url", problemUrl.trimmed());
    if (!query.exec()) {
        return tags;
    }
    while (query.next()) {
        tags << query.value(0).toString();
    }
    return tags;
}

QStringList ProblemMetaRepository::loadAllTags() const
{
    QStringList tags;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return tags;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec("SELECT name FROM tags ORDER BY name ASC")) {
        return tags;
    }
    while (query.next()) {
        tags << query.value(0).toString();
    }
    return tags;
}

QHash<QString, int> ProblemMetaRepository::statusCounts() const
{
    QHash<QString, int> counts;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return counts;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT task_status, COUNT(*) FROM problem_meta GROUP BY task_status")) {
        return counts;
    }
    while (query.next()) {
        counts.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return counts;
}

QHash<QString, int> ProblemMetaRepository::tagCounts() const
{
    QHash<QString, int> counts;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return counts;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT t.name, COUNT(pt.problem_url) FROM tags t "
            "JOIN problem_tags pt ON pt.tag_id = t.id "
            "GROUP BY t.id, t.name ORDER BY COUNT(pt.problem_url) DESC, t.name ASC")) {
        return counts;
    }
    while (query.next()) {
        counts.insert(query.value(0).toString(), query.value(1).toInt());
    }
    return counts;
}

QList<ProblemMeta> ProblemMetaRepository::reviewProblems() const
{
    QList<ProblemMeta> result;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return result;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT problem_url, title, note, difficulty, task_status, priority, deadline, review_flag, "
            "review_interval, review_ease, next_review_at, last_reviewed_at, review_count "
            "FROM problem_meta WHERE review_flag = 1 ORDER BY updated_at DESC")) {
        return result;
    }
    while (query.next()) {
        ProblemMeta meta = readMeta(query);
        meta.tags = loadTags(meta.problemUrl);
        result << meta;
    }
    return result;
}

int ProblemMetaRepository::notesCount() const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return 0;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT COUNT(*) FROM problem_meta WHERE note IS NOT NULL AND TRIM(note) <> ''")
        || !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

QList<ProblemMeta> ProblemMetaRepository::dueReviewProblems(const QString &nowIso) const
{
    QList<ProblemMeta> result;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return result;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT problem_url, title, note, difficulty, task_status, priority, deadline, review_flag, "
        "review_interval, review_ease, next_review_at, last_reviewed_at, review_count "
        "FROM problem_meta "
        "WHERE review_flag = 1 AND next_review_at IS NOT NULL AND next_review_at <> '' "
        "AND next_review_at <= :now "
        "ORDER BY next_review_at ASC");
    query.bindValue(":now", nowIso);
    if (!query.exec()) {
        return result;
    }
    while (query.next()) {
        ProblemMeta meta = readMeta(query);
        meta.tags = loadTags(meta.problemUrl);
        result << meta;
    }
    return result;
}

int ProblemMetaRepository::dueReviewCount(const QString &nowIso) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return 0;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT COUNT(*) FROM problem_meta "
        "WHERE review_flag = 1 AND next_review_at IS NOT NULL AND next_review_at <> '' "
        "AND next_review_at <= :now");
    query.bindValue(":now", nowIso);
    if (!query.exec() || !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

bool ProblemMetaRepository::updateReviewSchedule(const QString &problemUrl,
                                                 int interval,
                                                 int ease,
                                                 const QString &nextReviewAt,
                                                 const QString &lastReviewedAt,
                                                 int reviewCount)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "UPDATE problem_meta SET "
        "review_interval = :interval, review_ease = :ease, next_review_at = :next, "
        "last_reviewed_at = :last, review_count = :count "
        "WHERE problem_url = :problem_url");
    query.bindValue(":interval", interval);
    query.bindValue(":ease", ease);
    query.bindValue(":next", nextReviewAt);
    query.bindValue(":last", lastReviewedAt);
    query.bindValue(":count", reviewCount);
    query.bindValue(":problem_url", problemUrl.trimmed());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProblemMetaRepository::ensureReviewScheduled(const QString &problemUrl,
                                                  const QString &nowIso)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "UPDATE problem_meta SET "
        "next_review_at = :now, review_interval = 0, review_ease = 250, review_count = 0 "
        "WHERE problem_url = :problem_url AND review_flag = 1 "
        "AND (next_review_at IS NULL OR next_review_at = '')");
    query.bindValue(":now", nowIso);
    query.bindValue(":problem_url", problemUrl.trimmed());
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QString ProblemMetaRepository::lastError() const
{
    return m_lastError;
}

bool ProblemMetaRepository::openDatabase()
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase database =
            QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
        database.setDatabaseName(databasePath());
        if (!database.open()) {
            m_lastError = database.lastError().text();
            return false;
        }
        return true;
    }

    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    if (database.isOpen()) {
        return true;
    }

    const bool ok = database.open();
    if (!ok) {
        m_lastError = database.lastError().text();
    }
    return ok;
}

ProblemMeta ProblemMetaRepository::readMeta(const QSqlQuery &query) const
{
    ProblemMeta meta;
    meta.problemUrl = query.value(0).toString();
    meta.title = query.value(1).toString();
    meta.note = query.value(2).toString();
    meta.difficulty = query.value(3).toInt();
    meta.taskStatus = query.value(4).toString();
    meta.priority = query.value(5).toInt();
    meta.deadline = query.value(6).toString();
    meta.reviewFlag = query.value(7).toInt() != 0;
    meta.reviewInterval = query.value(8).toInt();
    meta.reviewEase = query.value(9).toInt();
    meta.nextReviewAt = query.value(10).toString();
    meta.lastReviewedAt = query.value(11).toString();
    meta.reviewCount = query.value(12).toInt();
    return meta;
}
