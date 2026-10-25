#include "config/appconfig.h"

#include "config/apppaths.h"

#include "../../toml++/toml.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace
{
bool loadDocument(toml::table &document, QString *errorMessage = nullptr)
{
    const QString path = AppPaths::appconfigPath();
    QFile file(path);
    if (!file.exists()) {
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to read appconfig.toml: %1")
                                .arg(file.errorString());
        }
        return false;
    }

    const QByteArray contents = file.readAll();
    try {
        document = toml::parse(
            std::string_view(contents.constData(), static_cast<size_t>(contents.size())),
            path.toUtf8().toStdString());
    } catch (const toml::parse_error &error) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Invalid appconfig.toml: %1")
                                .arg(QString::fromUtf8(error.what()));
        }
        return false;
    }

    return true;
}

QString readString(const toml::table &document,
                   std::string_view sectionName,
                   std::string_view key,
                   const QString &defaultValue = QString())
{
    const toml::table *section = document.get_as<toml::table>(sectionName);
    if (section == nullptr) {
        return defaultValue;
    }

    const std::optional<std::string> value = (*section)[key].value<std::string>();
    if (!value.has_value()) {
        return defaultValue;
    }
    return QString::fromUtf8(value->data(), static_cast<qsizetype>(value->size()));
}

bool saveDocument(const toml::table &document, QString *errorMessage)
{
    const QString path = AppPaths::appconfigPath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to create appconfig directory: %1")
                                .arg(QFileInfo(path).absolutePath());
        }
        return false;
    }

    std::ostringstream stream;
    stream << document;
    const QByteArray contents = QByteArray::fromStdString(stream.str());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write appconfig.toml: %1")
                                .arg(file.errorString());
        }
        return false;
    }
    if (file.write(contents) != contents.size()) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to write appconfig.toml: %1")
                                .arg(file.errorString());
        }
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage != nullptr) {
            *errorMessage = QString("Failed to save appconfig.toml: %1")
                                .arg(file.errorString());
        }
        return false;
    }
    return true;
}

toml::table &ensureSection(toml::table &document, std::string_view sectionName)
{
    toml::table *section = document.get_as<toml::table>(sectionName);
    if (section == nullptr) {
        document.insert_or_assign(sectionName, toml::table{});
        section = document.get_as<toml::table>(sectionName);
    }
    return *section;
}

bool saveString(std::string_view sectionName,
                std::string_view key,
                const QString &value,
                QString *errorMessage)
{
    toml::table document;
    if (!loadDocument(document, errorMessage)) {
        return false;
    }

    const QByteArray utf8 = value.toUtf8();
    ensureSection(document, sectionName)
        .insert_or_assign(key, std::string(utf8.constData(),
                                           static_cast<size_t>(utf8.size())));
    return saveDocument(document, errorMessage);
}
}

bool AppConfig::loadAlarmEnabled(bool defaultValue)
{
    toml::table document;
    if (!loadDocument(document)) {
        return defaultValue;
    }

    const toml::table *alarm = document.get_as<toml::table>("alarm");
    if (alarm == nullptr) {
        return defaultValue;
    }
    return (*alarm)["enabled"].value<bool>().value_or(defaultValue);
}

QString AppConfig::loadRingPath()
{
    toml::table document;
    if (!loadDocument(document)) {
        return QString();
    }
    return readString(document, "alarm", "ring_path");
}

QString AppConfig::loadServerUrl()
{
    toml::table document;
    if (!loadDocument(document)) {
        return QString();
    }
    return readString(document, "network", "ServerUrl");
}

bool AppConfig::saveAlarmEnabled(bool enabled, QString *errorMessage)
{
    toml::table document;
    if (!loadDocument(document, errorMessage)) {
        return false;
    }

    ensureSection(document, "alarm").insert_or_assign("enabled", enabled);
    return saveDocument(document, errorMessage);
}

bool AppConfig::saveServerUrl(const QString &value, QString *errorMessage)
{
    return saveString("network", "ServerUrl", value, errorMessage);
}

bool AppConfig::saveRingPath(const QString &path, QString *errorMessage)
{
    return saveString("alarm", "ring_path", path, errorMessage);
}
