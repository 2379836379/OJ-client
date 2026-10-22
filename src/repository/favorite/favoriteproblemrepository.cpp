#include "favoriteproblemrepository.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace
{
QString databasePath()
{
    return QDir(AppPaths::dataDir()).filePath("favorites.db");
}

bool ensureColumn(QSqlDatabase &database,
                  const QString &tableName,
                  const QString &columnName,
                  const QString &definition,
                  QString *error)
{
    QSqlQuery infoQuery(database);
    if (!infoQuery.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
        if (error != nullptr) {
            *error = infoQuery.lastError().text();
        }
        return false;
    }

    while (infoQuery.next()) {
        if (infoQuery.value(1).toString() == columnName) {
            return true;
        }
    }

    QSqlQuery alterQuery(database);
    const bool ok = alterQuery.exec(
        QString("ALTER TABLE %1 ADD COLUMN %2 %3")
            .arg(tableName, columnName, definition));
    if (!ok && error != nullptr) {
        *error = alterQuery.lastError().text();
    }
    return ok;
}
}

FavoriteProblemRepository::FavoriteProblemRepository()
    : m_connectionName(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

FavoriteProblemRepository::~FavoriteProblemRepository()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName);
        database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool FavoriteProblemRepository::initialize()
{
    if (!openDatabase()) {
        return false;
    }

    return ensureSchema();
}

bool FavoriteProblemRepository::ensureSchema()
{
    QSqlDatabase database = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(database);
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS favorite_folders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS favorite_problems ("
        "problem_url TEXT PRIMARY KEY,"
        "title TEXT NOT NULL,"
        "submit_url TEXT,"
        "time_limit TEXT,"
        "memory_limit TEXT,"
        "description TEXT,"
        "starter_code TEXT,"
        "input_spec TEXT,"
        "output_spec TEXT,"
        "sample_input TEXT,"
        "sample_output TEXT,"
        "hint TEXT,"
        "saved_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (!ensureColumn(database, "favorite_problems", "starter_code", "TEXT", &m_lastError)) {
        return false;
    }
    if (!ensureColumn(database, "favorite_problems", "input_spec", "TEXT", &m_lastError)) {
        return false;
    }
    if (!ensureColumn(database, "favorite_problems", "output_spec", "TEXT", &m_lastError)) {
        return false;
    }
    if (!ensureColumn(database, "favorite_problems", "sample_input", "TEXT", &m_lastError)) {
        return false;
    }
    if (!ensureColumn(database, "favorite_problems", "sample_output", "TEXT", &m_lastError)) {
        return false;
    }
    if (!ensureColumn(database, "favorite_problems", "hint", "TEXT", &m_lastError)) {
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS favorite_folder_items ("
        "folder_id INTEGER NOT NULL,"
        "problem_url TEXT NOT NULL,"
        "saved_at TEXT DEFAULT CURRENT_TIMESTAMP,"
        "PRIMARY KEY(folder_id, problem_url),"
        "FOREIGN KEY(folder_id) REFERENCES favorite_folders(id) ON DELETE CASCADE,"
        "FOREIGN KEY(problem_url) REFERENCES favorite_problems(problem_url) ON DELETE CASCADE"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS problem_translations ("
        "problem_url TEXT PRIMARY KEY,"
        "description TEXT,"
        "input_spec TEXT,"
        "output_spec TEXT,"
        "hint TEXT,"
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")");
    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool FavoriteProblemRepository::createFolder(const QString &folderName, qint64 *folderId)
{
    if (!openDatabase()) {
        return false;
    }

    const QString normalizedName = folderName.trimmed();
    if (normalizedName.isEmpty()) {
        m_lastError = "Folder name cannot be empty";
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO favorite_folders (name, created_at, updated_at) "
        "VALUES (:name, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");
    query.bindValue(":name", normalizedName);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (folderId != nullptr) {
        *folderId = query.lastInsertId().toLongLong();
    }
    return true;
}

bool FavoriteProblemRepository::removeFolder(qint64 folderId)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("DELETE FROM favorite_folders WHERE id = :folder_id");
    query.bindValue(":folder_id", folderId);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

QList<FavoriteFolderInfo> FavoriteProblemRepository::loadFolders() const
{
    QList<FavoriteFolderInfo> folders;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return folders;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(
            "SELECT f.id, f.name, COUNT(i.problem_url) "
            "FROM favorite_folders f "
            "LEFT JOIN favorite_folder_items i ON i.folder_id = f.id "
            "GROUP BY f.id, f.name "
            "ORDER BY f.name ASC")) {
        return folders;
    }

    while (query.next()) {
        FavoriteFolderInfo info;
        info.id = query.value(0).toLongLong();
        info.name = query.value(1).toString();
        info.problemCount = query.value(2).toInt();
        folders << info;
    }

    return folders;
}

QList<ProblemPageInfo> FavoriteProblemRepository::loadFavoritesInFolder(qint64 folderId) const
{
    QList<ProblemPageInfo> favorites;
    if (!QSqlDatabase::contains(m_connectionName)) {
        return favorites;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT p.problem_url, p.title, p.submit_url, p.time_limit, p.memory_limit, "
        "p.description, p.starter_code, p.input_spec, p.output_spec, p.sample_input, p.sample_output, p.hint "
        "FROM favorite_folder_items i "
        "JOIN favorite_problems p ON p.problem_url = i.problem_url "
        "WHERE i.folder_id = :folder_id "
        "ORDER BY i.saved_at DESC, p.title ASC");
    query.bindValue(":folder_id", folderId);
    if (!query.exec()) {
        return favorites;
    }

    while (query.next()) {
        favorites << readProblem(query);
    }

    return favorites;
}

bool FavoriteProblemRepository::saveFavoriteToFolder(const ProblemPageInfo &problemPageInfo,
                                                     qint64 folderId)
{
    if (!openDatabase()) {
        return false;
    }

    if (!upsertProblem(problemPageInfo)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO favorite_folder_items (folder_id, problem_url, saved_at) "
        "VALUES (:folder_id, :problem_url, CURRENT_TIMESTAMP) "
        "ON CONFLICT(folder_id, problem_url) DO UPDATE SET "
        "saved_at = CURRENT_TIMESTAMP");
    query.bindValue(":folder_id", folderId);
    query.bindValue(":problem_url", problemPageInfo.problemUrl);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

bool FavoriteProblemRepository::removeFavoriteFromFolder(const QString &problemUrl,
                                                         qint64 folderId)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "DELETE FROM favorite_folder_items "
        "WHERE folder_id = :folder_id AND problem_url = :problem_url");
    query.bindValue(":folder_id", folderId);
    query.bindValue(":problem_url", problemUrl);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

bool FavoriteProblemRepository::upsertProblem(const ProblemPageInfo &problemPageInfo)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO favorite_problems ("
        "problem_url, title, submit_url, time_limit, memory_limit, "
        "description, starter_code, input_spec, output_spec, sample_input, sample_output, hint, saved_at"
        ") VALUES ("
        ":problem_url, :title, :submit_url, :time_limit, :memory_limit, "
        ":description, :starter_code, :input_spec, :output_spec, :sample_input, :sample_output, :hint, CURRENT_TIMESTAMP"
        ") "
        "ON CONFLICT(problem_url) DO UPDATE SET "
        "title = excluded.title, "
        "submit_url = excluded.submit_url, "
        "time_limit = excluded.time_limit, "
        "memory_limit = excluded.memory_limit, "
        "description = excluded.description, "
        "starter_code = excluded.starter_code, "
        "input_spec = excluded.input_spec, "
        "output_spec = excluded.output_spec, "
        "sample_input = excluded.sample_input, "
        "sample_output = excluded.sample_output, "
        "hint = excluded.hint, "
        "saved_at = CURRENT_TIMESTAMP");
    query.bindValue(":problem_url", problemPageInfo.problemUrl);
    query.bindValue(":title", problemPageInfo.title);
    query.bindValue(":submit_url", problemPageInfo.submitUrl);
    query.bindValue(":time_limit", problemPageInfo.timeLimit);
    query.bindValue(":memory_limit", problemPageInfo.memoryLimit);
    query.bindValue(":description", problemPageInfo.description);
    query.bindValue(":starter_code", problemPageInfo.starterCode);
    query.bindValue(":input_spec", problemPageInfo.inputSpec);
    query.bindValue(":output_spec", problemPageInfo.outputSpec);
    query.bindValue(":sample_input", problemPageInfo.sampleInput);
    query.bindValue(":sample_output", problemPageInfo.sampleOutput);
    query.bindValue(":hint", problemPageInfo.hint);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

bool FavoriteProblemRepository::loadFavorite(
    const QString &problemUrl, ProblemPageInfo *problemPageInfo) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT problem_url, title, submit_url, time_limit, memory_limit, "
        "description, starter_code, input_spec, output_spec, sample_input, sample_output, hint "
        "FROM favorite_problems WHERE problem_url = :problem_url");
    query.bindValue(":problem_url", problemUrl);
    if (!query.exec() || !query.next()) {
        return false;
    }

    if (problemPageInfo != nullptr) {
        *problemPageInfo = readProblem(query);
    }
    return true;
}

bool FavoriteProblemRepository::saveProblemTranslation(
    const QString &problemUrl, const ProblemTranslationInfo &translationInfo)
{
    if (!openDatabase()) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO problem_translations ("
        "problem_url, description, input_spec, output_spec, hint, updated_at"
        ") VALUES ("
        ":problem_url, :description, :input_spec, :output_spec, :hint, CURRENT_TIMESTAMP"
        ") "
        "ON CONFLICT(problem_url) DO UPDATE SET "
        "description = excluded.description, "
        "input_spec = excluded.input_spec, "
        "output_spec = excluded.output_spec, "
        "hint = excluded.hint, "
        "updated_at = CURRENT_TIMESTAMP");
    query.bindValue(":problem_url", problemUrl);
    query.bindValue(":description", translationInfo.description);
    query.bindValue(":input_spec", translationInfo.inputSpec);
    query.bindValue(":output_spec", translationInfo.outputSpec);
    query.bindValue(":hint", translationInfo.hint);

    const bool ok = query.exec();
    if (!ok) {
        m_lastError = query.lastError().text();
    }
    return ok;
}

bool FavoriteProblemRepository::loadProblemTranslation(
    const QString &problemUrl, ProblemTranslationInfo *translationInfo) const
{
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT description, input_spec, output_spec, hint "
        "FROM problem_translations WHERE problem_url = :problem_url");
    query.bindValue(":problem_url", problemUrl);
    if (!query.exec() || !query.next()) {
        return false;
    }

    if (translationInfo != nullptr) {
        translationInfo->description = query.value(0).toString();
        translationInfo->inputSpec = query.value(1).toString();
        translationInfo->outputSpec = query.value(2).toString();
        translationInfo->hint = query.value(3).toString();
    }
    return true;
}

QString FavoriteProblemRepository::lastError() const
{
    return m_lastError;
}

bool FavoriteProblemRepository::openDatabase()
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

ProblemPageInfo FavoriteProblemRepository::readProblem(const QSqlQuery &query) const
{
    ProblemPageInfo info;
    info.problemUrl = query.value(0).toString();
    info.title = query.value(1).toString();
    info.submitUrl = query.value(2).toString();
    info.timeLimit = query.value(3).toString();
    info.memoryLimit = query.value(4).toString();
    info.description = query.value(5).toString();
    info.starterCode = query.value(6).toString();
    info.inputSpec = query.value(7).toString();
    info.outputSpec = query.value(8).toString();
    info.sampleInput = query.value(9).toString();
    info.sampleOutput = query.value(10).toString();
    info.hint = query.value(11).toString();
    return info;
}
