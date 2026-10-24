#pragma once

#include <QByteArray>
#include <QString>

namespace GroupHomepageParser
{
QString parseGroupHomepage(const QByteArray &html);
}
