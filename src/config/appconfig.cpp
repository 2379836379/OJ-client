#include "config/appconfig.h"

#include "config/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>

namespace
{
QString unquoteTomlValue(const QString &value)
{
    QString trimmed = value.trimmed();
    if (trimmed.startsWith('"') && trimmed.endsWith('"') && trimmed.size() >= 2) {
        trimmed = trimmed.mid(1, trimmed.size() - 2);
    }
    trimmed.replace("\\n", "\n");
    trimmed.replace("\\\"", "\"");
    return trimmed;
}

QStringList candidateConfigPaths()
{
    return {AppPaths::configFilePath()};
}

QString configOutputPath()
{
    return AppPaths::configFilePath();
}

QString appStateOutputPath()
{
    return AppPaths::appStateFilePath();
}

QString readAppStateRootString(const QString &targetKey)
{
    const QString loadedPath = appStateOutputPath();
    if (!QFile::exists(loadedPath)) {
        return QString();
    }

    QFile file(loadedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    const QString content = QString::fromUtf8(file.readAll());
    QString currentSection;
    const QRegularExpression sectionRegex("^\\s*\\[([^\\]]+)\\]\\s*$");
    const QRegularExpression keyValueRegex("^\\s*([A-Za-z0-9_\\-]+)\\s*=\\s*(.*?)\\s*$");

    const QStringList lines = content.split('\n');
    for (QString line : lines) {
        const int commentIndex = line.indexOf('#');
        if (commentIndex >= 0) {
            line = line.left(commentIndex);
        }
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch sectionMatch = sectionRegex.match(line);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1).trimmed().toLower();
            continue;
        }

        if (!currentSection.isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch keyValueMatch = keyValueRegex.match(line);
        if (!keyValueMatch.hasMatch()) {
            continue;
        }

        if (keyValueMatch.captured(1).trimmed().toLower() == targetKey.toLower()) {
            return unquoteTomlValue(keyValueMatch.captured(2));
        }
    }

    return QString();
}

QString quoteTomlValue(QString text)
{
    text.replace("\\", "\\\\");
    text.replace("\"", "\\\"");
    text.replace("\n", "\\n");
    return QString("\"%1\"").arg(text);
}

QString quoteTomlBool(bool value)
{
    return value ? "true" : "false";
}

QString updateOrAppendRootKey(const QString &content,
                              const QString &key,
                              const QString &value)
{
    const QStringList lines = content.split('\n');
    QStringList output;
    bool replaced = false;
    bool inserted = false;
    bool inRootSection = true;

    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        const bool isSection = trimmed.startsWith('[') && trimmed.endsWith(']');
        if (isSection && inRootSection && !inserted && !replaced) {
            output << QString("%1 = %2").arg(key, value);
            inserted = true;
        }
        if (isSection) {
            inRootSection = false;
        }

        const QRegularExpression regex(
            QString("^\\s*%1\\s*=.*$").arg(QRegularExpression::escape(key)));
        if (inRootSection && regex.match(line).hasMatch()) {
            output << QString("%1 = %2").arg(key, value);
            replaced = true;
            continue;
        }

        output << line;
    }

    if (!replaced && !inserted) {
        if (!output.isEmpty() && !output.last().isEmpty()) {
            output << QString();
        }
        output << QString("%1 = %2").arg(key, value);
    }

    return output.join('\n');
}

bool writeAppStateRootKey(const QString &key, const QString &value, QString *errorMessage)
{
    const QString savedPath = appStateOutputPath();
    QDir().mkpath(QFileInfo(savedPath).absolutePath());

    QString content;
    QFile inputFile(savedPath);
    if (inputFile.exists()) {
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage != nullptr) {
                *errorMessage = QString("Failed to read appstate.toml: %1")
                                    .arg(inputFile.errorString());
            }
            return false;
        }
        content = QString::fromUtf8(inputFile.readAll());
        inputFile.close();
    }

    const QString updatedContent = updateOrAppendRootKey(content, key, quoteTomlValue(value));

    QFile outputFile(savedPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write appstate.toml: %1")
                                .arg(outputFile.errorString());
        }
        return false;
    }
    outputFile.write(updatedContent.toUtf8());
    outputFile.close();
    return true;
}
}

OpenAiConfig AppConfig::loadOpenAiConfig()
{
    OpenAiConfig config;
    QString loadedPath;
    for (const QString &path : candidateConfigPaths()) {
        if (QFile::exists(path)) {
            loadedPath = QDir::cleanPath(path);
            break;
        }
    }
    if (loadedPath.isEmpty()) {
        return config;
    }

    QFile file(loadedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return config;
    }

    const QString content = QString::fromUtf8(file.readAll());
    QString currentSection;
    QString modelProvider;
    QHash<QString, QString> providerBaseUrls;
    QHash<QString, QString> providerEnvKeys;
    const QRegularExpression sectionRegex("^\\s*\\[([^\\]]+)\\]\\s*$");
    const QRegularExpression keyValueRegex("^\\s*([A-Za-z0-9_\\-]+)\\s*=\\s*(.*?)\\s*$");

    const QStringList lines = content.split('\n');
    for (QString line : lines) {
        const int commentIndex = line.indexOf('#');
        if (commentIndex >= 0) {
            line = line.left(commentIndex);
        }
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch sectionMatch = sectionRegex.match(line);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1).trimmed().toLower();
            continue;
        }

        const QRegularExpressionMatch keyValueMatch = keyValueRegex.match(line);
        if (!keyValueMatch.hasMatch()) {
            continue;
        }

        const QString key = keyValueMatch.captured(1).trimmed().toLower();
        const QString value = unquoteTomlValue(keyValueMatch.captured(2));

        if (currentSection.isEmpty()) {
            if (key == "model") {
                if (!value.isEmpty()) {
                    config.model = value;
                }
            } else if (key == "system_prompt") {
                config.systemPrompt = value;
            } else if (key == "api_key") {
                config.apiKey = value;
            } else if (key == "base_url") {
                const QUrl baseUrl(value);
                if (baseUrl.isValid() && !baseUrl.scheme().isEmpty()) {
                    config.baseUrl = baseUrl;
                }
            } else if (key == "model_provider") {
                modelProvider = value.trimmed().toLower();
            }
            continue;
        }

        if (currentSection == "openai") {
            if (key == "base_url") {
                const QUrl baseUrl(value);
                if (baseUrl.isValid() && !baseUrl.scheme().isEmpty()) {
                    config.baseUrl = baseUrl;
                }
            } else if (key == "model") {
                if (!value.isEmpty()) {
                    config.model = value;
                }
            } else if (key == "system_prompt") {
                config.systemPrompt = value;
            } else if (key == "api_key") {
                config.apiKey = value;
            }
            continue;
        }

        if (currentSection.startsWith("model_providers.")) {
            const QString providerName = currentSection.mid(QString("model_providers.").size());
            if (providerName.isEmpty()) {
                continue;
            }
            if (key == "base_url") {
                providerBaseUrls.insert(providerName, value);
            } else if (key == "env_key") {
                providerEnvKeys.insert(providerName, value);
            }
        }
    }

    if (!modelProvider.isEmpty()) {
        const QString providerName = modelProvider.toLower();
        const QString providerBaseUrl = providerBaseUrls.value(providerName);
        if (!providerBaseUrl.isEmpty()) {
            const QUrl baseUrl(providerBaseUrl);
            if (baseUrl.isValid() && !baseUrl.scheme().isEmpty()) {
                config.baseUrl = baseUrl;
            }
        }
        if (config.apiKey.trimmed().isEmpty()) {
            config.apiKey = providerEnvKeys.value(providerName);
        }
    }

    config.sourcePath = loadedPath;
    return config;
}

QString AppConfig::loadRingPath()
{
    const QString loadedPath = appStateOutputPath();
    if (!QFile::exists(loadedPath)) {
        return QString();
    }

    QFile file(loadedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    const QString content = QString::fromUtf8(file.readAll());
    QString currentSection;
    const QRegularExpression sectionRegex("^\\s*\\[([^\\]]+)\\]\\s*$");
    const QRegularExpression keyValueRegex("^\\s*([A-Za-z0-9_\\-]+)\\s*=\\s*(.*?)\\s*$");

    const QStringList lines = content.split('\n');
    for (QString line : lines) {
        const int commentIndex = line.indexOf('#');
        if (commentIndex >= 0) {
            line = line.left(commentIndex);
        }
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch sectionMatch = sectionRegex.match(line);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1).trimmed().toLower();
            continue;
        }

        if (!currentSection.isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch keyValueMatch = keyValueRegex.match(line);
        if (!keyValueMatch.hasMatch()) {
            continue;
        }

        const QString key = keyValueMatch.captured(1).trimmed().toLower();
        if (key == "ring_path") {
            return unquoteTomlValue(keyValueMatch.captured(2));
        }
    }

    return QString();
}

QString AppConfig::loadJudgerBaseUrl(const QString &defaultValue)
{
    const QString value = readAppStateRootString("judger_base_url").trimmed();
    return value.isEmpty() ? defaultValue : value;
}

QString AppConfig::loadEmailVerifyUrl(const QString &defaultValue)
{
    const QString value = readAppStateRootString("email_verify_url").trimmed();
    return value.isEmpty() ? defaultValue : value;
}

QString AppConfig::loadOpenJudgeBaseUrl(const QString &defaultValue)
{
    const QString value = readAppStateRootString("openjudge_base_url").trimmed();
    return value.isEmpty() ? defaultValue : value;
}

bool AppConfig::loadAlarmEnabled(bool defaultValue)
{
    const QString loadedPath = appStateOutputPath();
    if (!QFile::exists(loadedPath)) {
        return defaultValue;
    }

    QFile file(loadedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return defaultValue;
    }

    const QString content = QString::fromUtf8(file.readAll());
    QString currentSection;
    const QRegularExpression sectionRegex("^\\s*\\[([^\\]]+)\\]\\s*$");
    const QRegularExpression keyValueRegex("^\\s*([A-Za-z0-9_\\-]+)\\s*=\\s*(.*?)\\s*$");

    const QStringList lines = content.split('\n');
    for (QString line : lines) {
        const int commentIndex = line.indexOf('#');
        if (commentIndex >= 0) {
            line = line.left(commentIndex);
        }
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch sectionMatch = sectionRegex.match(line);
        if (sectionMatch.hasMatch()) {
            currentSection = sectionMatch.captured(1).trimmed().toLower();
            continue;
        }

        if (!currentSection.isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch keyValueMatch = keyValueRegex.match(line);
        if (!keyValueMatch.hasMatch()) {
            continue;
        }

        const QString key = keyValueMatch.captured(1).trimmed().toLower();
        if (key != "alarm_enabled") {
            continue;
        }

        const QString value = keyValueMatch.captured(2).trimmed().toLower();
        if (value == "true") {
            return true;
        }
        if (value == "false") {
            return false;
        }
        return defaultValue;
    }

    return defaultValue;
}

QString AppConfig::loadConfigText(QString *sourcePath)
{
    QString loadedPath;
    for (const QString &path : candidateConfigPaths()) {
        if (QFile::exists(path)) {
            loadedPath = QDir::cleanPath(path);
            break;
        }
    }
    if (sourcePath != nullptr) {
        *sourcePath = loadedPath.isEmpty() ? configOutputPath() : loadedPath;
    }
    if (loadedPath.isEmpty()) {
        return QString(
            "[openai]\n"
            "base_url = \"https://api.openai.com/v1\"\n"
            "model = \"gpt-5-mini\"\n"
            "system_prompt = \"You are an AI programming assistant for algorithm problems.\"\n"
            "api_key = \"\"\n");
    }

    QFile file(loadedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

bool AppConfig::saveOpenAiConfig(const OpenAiConfig &config, QString *errorMessage)
{
    const QString outputPath = configOutputPath();
    QDir().mkpath(QFileInfo(outputPath).absolutePath());

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write config.toml: %1").arg(file.errorString());
        }
        return false;
    }

    const QString content = QString(
                                "[openai]\n"
                                "base_url = %1\n"
                                "model = %2\n"
                                "system_prompt = %3\n"
                                "api_key = %4\n")
                                .arg(quoteTomlValue(config.baseUrl.toString()),
                                     quoteTomlValue(config.model),
                                     quoteTomlValue(config.systemPrompt),
                                     quoteTomlValue(config.apiKey));
    file.write(content.toUtf8());
    file.close();
    return true;
}

bool AppConfig::saveRingPath(const QString &path, QString *errorMessage)
{
    const QString savedPath = appStateOutputPath();
    QDir().mkpath(QFileInfo(savedPath).absolutePath());

    QString content;
    QFile inputFile(savedPath);
    if (inputFile.exists()) {
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage != nullptr) {
                *errorMessage = QString("Failed to read appstate.toml: %1")
                                    .arg(inputFile.errorString());
            }
            return false;
        }
        content = QString::fromUtf8(inputFile.readAll());
        inputFile.close();
    }

    const QString updatedContent = updateOrAppendRootKey(
        content, "ring_path", quoteTomlValue(path));

    QFile outputFile(savedPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write appstate.toml: %1")
                                .arg(outputFile.errorString());
        }
        return false;
    }
    outputFile.write(updatedContent.toUtf8());
    outputFile.close();
    return true;
}

bool AppConfig::saveAlarmEnabled(bool enabled, QString *errorMessage)
{
    const QString savedPath = appStateOutputPath();
    QDir().mkpath(QFileInfo(savedPath).absolutePath());

    QString content;
    QFile inputFile(savedPath);
    if (inputFile.exists()) {
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage != nullptr) {
                *errorMessage = QString("Failed to read appstate.toml: %1")
                                    .arg(inputFile.errorString());
            }
            return false;
        }
        content = QString::fromUtf8(inputFile.readAll());
        inputFile.close();
    }

    const QString updatedContent = updateOrAppendRootKey(
        content, "alarm_enabled", quoteTomlBool(enabled));

    QFile outputFile(savedPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write appstate.toml: %1")
                                .arg(outputFile.errorString());
        }
        return false;
    }
    outputFile.write(updatedContent.toUtf8());
    outputFile.close();
    return true;
}

bool AppConfig::saveOpenJudgeBaseUrl(const QString &value, QString *errorMessage)
{
    return writeAppStateRootKey("openjudge_base_url", value.trimmed(), errorMessage);
}

bool AppConfig::saveJudgerBaseUrl(const QString &value, QString *errorMessage)
{
    return writeAppStateRootKey("judger_base_url", value.trimmed(), errorMessage);
}

bool AppConfig::saveEmailVerifyUrl(const QString &value, QString *errorMessage)
{
    return writeAppStateRootKey("email_verify_url", value.trimmed(), errorMessage);
}

bool AppConfig::saveConfigText(const QString &content,
                               QString *savedPath,
                               QString *errorMessage)
{
    const QString outputPath = configOutputPath();
    QDir().mkpath(QFileInfo(outputPath).absolutePath());

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write config.toml: %1").arg(file.errorString());
        }
        return false;
    }
    file.write(content.toUtf8());
    file.close();
    if (savedPath != nullptr) {
        *savedPath = outputPath;
    }
    return true;
}
