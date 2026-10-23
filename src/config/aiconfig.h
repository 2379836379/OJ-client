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

namespace AIConfig
{
OpenAiConfig loadOpenAiConfig();
QString loadConfigText(QString *sourcePath = nullptr);
bool saveOpenAiConfig(const OpenAiConfig &config, QString *errorMessage = nullptr);
bool saveConfigText(const QString &content,
                    QString *savedPath = nullptr,
                    QString *errorMessage = nullptr);
}
