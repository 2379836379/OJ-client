#pragma once

#include <QString>

class AppConfig
{
public:
    static bool loadAlarmEnabled(bool defaultValue = false);
    static QString loadRingPath();
    static QString loadServerUrl();
    static bool saveAlarmEnabled(bool enabled, QString *errorMessage = nullptr);
    static bool saveServerUrl(const QString &value, QString *errorMessage = nullptr);
    static bool saveRingPath(const QString &path, QString *errorMessage = nullptr);
};
