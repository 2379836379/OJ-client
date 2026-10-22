#pragma once

#include <QString>
#include <QUrl>

struct OpenAiConfig
{
    QUrl baseUrl = QUrl("https://api.openai.com/v1");
    QString model = "gpt-5-mini";
    QString systemPrompt;
    QString apiKey;
    QString sourcePath;
};

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
