#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct JoinedClassInfo
{
    QString name;
    QString url;
};

namespace LoginParser
{
QString extractPersonalHomeUrl(const QByteArray &html, const QUrl &baseUrl);
QList<JoinedClassInfo> extractJoinedClasses(const QByteArray &html,
                                            const QUrl &baseUrl);
}
