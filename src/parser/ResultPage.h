#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

struct ResultPageInfo
{
    QString pageUrl;
    QString solutionUrl;
    QString submissionId;
    QString statusText;
    QString statusClass;
    QString detailTitle;
    QString detailText;
    bool hasResult = false;
};

namespace ResultPageParser
{
ResultPageInfo parseResultPage(const QByteArray &html, const QUrl &baseUrl);
}
