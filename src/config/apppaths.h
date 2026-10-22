#pragma once

#include <QString>

// Centralized writable locations for user data (per-user AppData), replacing the
// previous "write next to the executable" layout that fails on read-only install
// directories (e.g. Windows Program Files).
namespace AppPaths
{
QString baseDir();           // <AppData>/oj-client
QString dataDir();           // baseDir/data   (databases, logs)
QString cacheDir();          // baseDir/cache  (regenerable cache databases)
QString configFilePath();    // baseDir/config.toml
QString appStateFilePath();  // baseDir/appstate.toml

// One-time copy of legacy data (key databases + toml) from the old executable
// directory into the new per-user location. Idempotent: only copies when the
// destination is missing. Cache databases are intentionally not migrated.
void migrateLegacyDataIfNeeded();
}
