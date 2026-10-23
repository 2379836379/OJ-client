#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>


namespace LoginParser
{
QString extractPersonalHomeUrl(const QByteArray &html, const QUrl &baseUrl);
}
