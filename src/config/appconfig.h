#pragma once

#include "config/aiconfig.h"

#include <QString>

class AppConfig
{
public:
    static OpenAiConfig loadOpenAiConfig();
    static bool loadAlarmEnabled(bool defaultValue = false);
    static QString loadRingPath();
    static QString loadJudgerBaseUrl(const QString &defaultValue);
    static QString loadEmailVerifyUrl(const QString &defaultValue);
    static QString loadOpenJudgeBaseUrl(const QString &defaultValue);
    static QString loadConfigText(QString *sourcePath = nullptr);
    static bool saveAlarmEnabled(bool enabled, QString *errorMessage = nullptr);
    static bool saveOpenJudgeBaseUrl(const QString &value, QString *errorMessage = nullptr);
    static bool saveJudgerBaseUrl(const QString &value, QString *errorMessage = nullptr);
    static bool saveEmailVerifyUrl(const QString &value, QString *errorMessage = nullptr);
    static bool saveOpenAiConfig(const OpenAiConfig &config, QString *errorMessage = nullptr);
    static bool saveRingPath(const QString &path, QString *errorMessage = nullptr);
    static bool saveConfigText(const QString &content,
                               QString *savedPath = nullptr,
                               QString *errorMessage = nullptr);
};
