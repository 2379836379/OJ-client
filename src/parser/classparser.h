#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

struct ClassPageInfo
{
    QString classPageUrl;
    QString groupEntryUrl = "not found";
    QString courseName = "not found";
};

namespace ClassParser
{
ClassPageInfo parseClassPage(const QByteArray &html, const QUrl &baseUrl);
}
