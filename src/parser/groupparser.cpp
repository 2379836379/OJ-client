#include "groupparser.h"

#include "parsercommon.h"

#include <QRegularExpression>

namespace
{
QString stripHtml(const QString &html)
{
    QString text = html;
    text.remove(QRegularExpression("<[^>]+>"));
    return text.simplified();
}
}

//
namespace GroupParser
{
GroupPageInfo parseGroupPage(const QByteArray &html, const QUrl &baseUrl)
{
    GroupPageInfo info;
    info.groupPageUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);
    const QRegularExpression listRegex(
        "<ul\\s+class=\"current-contest\\s+label\"[^>]*>([\\s\\S]*?)</ul>");
    const QRegularExpressionMatch listMatch = listRegex.match(text);
    if (!listMatch.hasMatch()) {
        return info;
    }

    const QString listHtml = listMatch.captured(1);
    const QRegularExpression itemRegex(
        "<li\\s+class=\"([^\"]*(?:contest-info|practice-info)[^\"]*)\"[^>]*>([\\s\\S]*?)</li>");
    QRegularExpressionMatchIterator itemIt = itemRegex.globalMatch(listHtml);
    while (itemIt.hasNext()) {
        const QRegularExpressionMatch itemMatch = itemIt.next();
        const QString itemClass = itemMatch.captured(1).trimmed();
        const QString itemHtml = itemMatch.captured(2);

        const QRegularExpression linkRegex(
            "<h3>[\\s\\S]*?<a\\s+href=\"([^\"]+)\">([^<]+)</a>([\\s\\S]*?)</h3>");
        const QRegularExpressionMatch linkMatch = linkRegex.match(itemHtml);
        if (!linkMatch.hasMatch()) {
            continue;
        }

        ContestSetInfo contestSet;
        contestSet.url =
            baseUrl.resolved(QUrl(linkMatch.captured(1))).toString();
        contestSet.title = linkMatch.captured(2).trimmed();
        contestSet.itemClass = itemClass;

        const QString total_problem = stripHtml(linkMatch.captured(3));
        if (!total_problem.isEmpty()) {
            contestSet.problemnumber = total_problem;
        }

        const QRegularExpression spanRegex(
            "<span\\s+class=\"([^\"]+)\"[^>]*>([\\s\\S]*?)</span>");
        QRegularExpressionMatchIterator spanIt = spanRegex.globalMatch(itemHtml);
        while (spanIt.hasNext()) {
            const QRegularExpressionMatch spanMatch = spanIt.next();
            const QString spanClass = spanMatch.captured(1).trimmed();
            const QString spanText = stripHtml(spanMatch.captured(2));
            if ((spanClass == "over-time" || spanClass == "recently-update")
                && !spanText.isEmpty()) {
                contestSet.endtime = spanText;
            }
        }
        info.contestSets.append(contestSet);
    }

    return info;
}
}
