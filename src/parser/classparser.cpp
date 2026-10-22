#include "classparser.h"

#include "parsercommon.h"

#include <QRegularExpression>



namespace ClassParser
{
ClassPageInfo parseClassPage(const QByteArray &html, const QUrl &baseUrl)
{
    ClassPageInfo info;
    info.classPageUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);
    const QRegularExpression groupEntryRegex(
        "<a\\s+href=\"([^\"]+)\"[^>]*>\\s*<img(?=[^>]*class=\"group-logo\")(?=[^>]*alt=\"([^\"]*)\")[^>]*>\\s*</a>\\s*"
        "<a\\s+href=\"[^\"]+\"[^>]*>\\s*前往小组\\s*</a>");
    const QRegularExpressionMatch groupEntryMatch = groupEntryRegex.match(text);
    if (groupEntryMatch.hasMatch()) {
        info.groupEntryUrl =
            baseUrl.resolved(QUrl(groupEntryMatch.captured(1))).toString();
        info.courseName = groupEntryMatch.captured(2).trimmed();
    }
    return info;
}
}
