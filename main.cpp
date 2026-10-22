#include "src/ui/mainwindow.h"

#include "src/config/apppaths.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QIcon>
#include <QTextStream>

namespace
{
QIcon loadAppIcon()
{
    QIcon icon(":/images/icon-page-1.ico");
    if (!icon.isNull()) {
        return icon;
    }

    QDir dir = QDir::current();
    const QStringList candidates = {
        dir.filePath("oj-client/images/icon-page-1.ico"),
        dir.filePath("images/icon-page-1.ico")
    };
    for (const QString &path : candidates) {
        QIcon fileIcon(path);
        if (!fileIcon.isNull()) {
            return fileIcon;
        }
    }
    return QIcon();
}

void writeStartupLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | " << message << '\n';
}
}

int main(int argc, char *argv[])
{
    writeStartupLog("main: entering");
    QApplication a(argc, argv);
    writeStartupLog("main: QApplication created");
    AppPaths::migrateLegacyDataIfNeeded();
    writeStartupLog("main: legacy data migration checked");
    QCoreApplication::setApplicationName(" ");
    QApplication::setApplicationDisplayName(" ");
    writeStartupLog("main: application name cleared");
    const QIcon appIcon = loadAppIcon();
    a.setWindowIcon(appIcon);
    writeStartupLog(QString("main: application icon set, null=%1")
                        .arg(appIcon.isNull() ? "true" : "false"));
    a.setFont(QFont("Microsoft YaHei UI", 10));
    writeStartupLog("main: application font set");
    QApplication::setQuitOnLastWindowClosed(false);
    writeStartupLog("main: quitOnLastWindowClosed set");
    MainWindow w;
    writeStartupLog("main: MainWindow constructed");
    w.show();
    writeStartupLog("main: MainWindow show called");
    return QCoreApplication::exec();
}
