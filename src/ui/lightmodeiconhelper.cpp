#include "ui/lightmodeiconhelper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <QWidget>

namespace
{
bool g_darkModeEnabled = false;

QString normalizeFileName(const QString &fileName, bool darkMode)
{
    if (!darkMode) {
        return fileName;
    }

    if (fileName.compare("dark-mode.png", Qt::CaseInsensitive) == 0) {
        return QStringLiteral("light-mode.svg");
    }
    if (fileName.compare("ai-config.svg", Qt::CaseInsensitive) == 0) {
        return QStringLiteral("aiconfig.svg");
    }
    if (fileName.compare("log out.svg", Qt::CaseInsensitive) == 0) {
        return QStringLiteral("logout.svg");
    }
    return fileName;
}

QString resolveIconPath(const QString &fileName, bool darkMode)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    const QString modeDir = darkMode ? QStringLiteral("dark_mode")
                                     : QStringLiteral("light_mode");
    const QString normalizedFileName = normalizeFileName(fileName, darkMode);
    const QStringList candidates = {
        QDir(cwd).filePath(QString("images/%1/%2").arg(modeDir, normalizedFileName)),
        QDir(appDir).filePath(QString("images/%1/%2").arg(modeDir, normalizedFileName)),
        QDir(appDir).filePath(QString("../images/%1/%2").arg(modeDir, normalizedFileName)),
        QDir(appDir).filePath(QString("../oj-client/images/%1/%2").arg(modeDir, normalizedFileName)),
        QDir(appDir).filePath(QString("../../oj-client/images/%1/%2").arg(modeDir, normalizedFileName))
    };

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            return QDir::cleanPath(path);
        }
    }
    return QString();
}

void applyResolvedIcon(QPushButton *button,
                       const QString &fileName,
                       bool iconOnly,
                       int iconSize)
{
    if (button == nullptr) {
        return;
    }

    QString path = resolveIconPath(fileName, g_darkModeEnabled);
    if (path.isEmpty() && g_darkModeEnabled) {
        path = resolveIconPath(fileName, false);
    }
    if (path.isEmpty()) {
        return;
    }

    button->setIcon(QIcon(path));
    button->setIconSize(QSize(iconSize, iconSize));
    if (iconOnly) {
        button->setText(QString());
        button->setToolTip(button->toolTip().isEmpty() ? fileName : button->toolTip());
        button->setMinimumWidth(iconSize + 18);
        button->setMaximumWidth(iconSize + 18);
    }
}
}

void LightModeIconHelper::setDarkModeEnabled(bool enabled)
{
    g_darkModeEnabled = enabled;
}

void LightModeIconHelper::applyIcon(QPushButton *button,
                                    const QString &fileName,
                                    bool iconOnly,
                                    int iconSize)
{
    if (button == nullptr) {
        return;
    }

    button->setProperty("_iconFileName", fileName);
    button->setProperty("_iconOnly", iconOnly);
    button->setProperty("_iconSize", iconSize);
    applyResolvedIcon(button, fileName, iconOnly, iconSize);
}

void LightModeIconHelper::refreshIcon(QPushButton *button)
{
    if (button == nullptr) {
        return;
    }

    const QString fileName = button->property("_iconFileName").toString();
    if (fileName.isEmpty()) {
        return;
    }

    applyResolvedIcon(button,
                      fileName,
                      button->property("_iconOnly").toBool(),
                      button->property("_iconSize").toInt());
}

void LightModeIconHelper::refreshIcons(QWidget *root)
{
    if (root == nullptr) {
        return;
    }

    const QList<QPushButton *> buttons = root->findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        refreshIcon(button);
    }
}

void LightModeIconHelper::applyToolsToggleIcon(QPushButton *button, bool expanded)
{
    if (button == nullptr) {
        return;
    }
    button->setIcon(QIcon());
    button->setText(expanded ? QStringLiteral("Tools v") : QStringLiteral(">"));
    button->setMinimumWidth(0);
    button->setMaximumWidth(QWIDGETSIZE_MAX);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    button->setStyleSheet(QString());
}
