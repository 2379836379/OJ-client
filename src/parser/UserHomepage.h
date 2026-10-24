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
QList<JoinedClassInfo> parseUserHomepage(const QByteArray &html);
}
