#include "classparser.h"

#include <QRegularExpression>



namespace ClassParser
{
// 小组首页 格式：<a href="http://noi.openjudge.cn/">前往小组</a>
QString parseClassPage(const QByteArray &html)
{
    const QString text = QString::fromUtf8(html);
    static const QRegularExpression groupEntryRegex("<a href=\"([^\"]+)\">前往小组</a>");
    const QRegularExpressionMatch groupEntryMatch = groupEntryRegex.match(text);
    if (!groupEntryMatch.hasMatch()) {
        return QString();
    }
    return groupEntryMatch.captured(1);
}
}
