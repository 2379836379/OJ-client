#pragma once

#include <QByteArray>
#include <QList>
#include <QString>


struct JoinedClassInfo
{
    QString name;
    QString url;
};

namespace UserHomepageParser
{
QList<JoinedClassInfo> parseUserHomepage(const QByteArray &html);
}
