#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace HomepageParser
{
QString parseHomepage(const QByteArray &html);
}
