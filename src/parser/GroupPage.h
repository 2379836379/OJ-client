#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct ContestSetInfo
{
    QString url;
    QString title;
    QString itemClass;
    QString problemnumber;
    QString endtime;
    QString extraText;
};

struct GroupPageInfo
{
    QString groupPageUrl;
    QList<ContestSetInfo> contestSets;
};

namespace GroupParser
{
GroupPageInfo parseGroupPage(const QByteArray &html, const QUrl &baseUrl);
}
