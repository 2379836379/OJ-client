#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

struct ResultPageInfo
{
    QString statusText;
    QString detailTitle;
    QString detailText;
    bool hasResult = false;
};

namespace ResultPageParser
{
ResultPageInfo parseResultPage(const QByteArray &html);
}
