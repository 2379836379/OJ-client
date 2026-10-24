#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct ContestSetInfo
{
    QString url;
    QString title;
    QString problemnumber;
    QString endtime;
};

struct GroupPageInfo
{
    QList<ContestSetInfo> contestSets;
};

namespace GroupPageParser
{
GroupPageInfo parseGroupPage(const QByteArray &html, const QUrl &baseUrl);
}
