#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>


struct JoinedClassInfo
{
    QString name;
    QString url;
};

namespace ClassParser
{
QList<JoinedClassInfo> extractJoinedClasses(const QByteArray &html);
QString parseClassPage(const QByteArray &html);
}
