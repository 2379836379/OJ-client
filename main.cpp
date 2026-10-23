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

QIcon loadAppIcon()
{
    QIcon icon(":/images/icon-page-1.ico");
    if (!icon.isNull()) {
        return icon;
    }
    return QIcon();
}
}

int main(int argc, char *argv[])
{
    writeStartupLog("main: entering");
    QApplication a(argc, argv);
    writeStartupLog("main: QApplication created");
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
