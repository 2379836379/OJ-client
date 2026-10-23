#pragma once

#include <QString>
// 集中管理的用户数据可写路径（每用户 AppData），
namespace AppPaths
{
QString baseDir();           // <AppData>/oj-client
QString dataDir();           // baseDir/data
QString logDir();            // baseDir/logs
QString cacheDir();          // baseDir/cache
QString AIconfigPath();    // baseDir/AIconfig.toml
QString appconfigPath();  // baseDir/appconfig.toml
}
