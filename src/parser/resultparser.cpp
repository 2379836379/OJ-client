#include "resultparser.h"

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

QString decodePreformattedText(const QString &html)
{
    QString text = html;
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&amp;", "&");
    text.replace("&quot;", "\"");
    text.replace("&#39;", "'");
    return text.trimmed();
}
}

namespace ResultParser
{
ResultPageInfo parseResultPage(const QByteArray &html, const QUrl &baseUrl)
{
    ResultPageInfo info;
    info.pageUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);

    const QRegularExpression titleRegex(
        "<div\\s+id=\"pageTitle\"[^>]*>\\s*<h2>\\s*#?(\\d+)\\s*提交状态\\s*</h2>\\s*</div>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch titleMatch = titleRegex.match(text);
    if (titleMatch.hasMatch()) {
        info.submissionId = titleMatch.captured(1).trimmed();
    }

    const QRegularExpression statusRegex(
        "<p\\s+class=\"compile-status\"[^>]*>[\\s\\S]*?状态:\\s*"
        "<a\\s+href=\"([^\"]+)\"\\s+class=\"([^\"]+)\"[^>]*>([\\s\\S]*?)</a>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch statusMatch = statusRegex.match(text);
    if (statusMatch.hasMatch()) {
        info.solutionUrl =
            baseUrl.resolved(QUrl(statusMatch.captured(1))).toString();
        info.statusClass = statusMatch.captured(2).trimmed();
        info.statusText = stripHtml(statusMatch.captured(3));
        info.hasResult = !info.statusText.isEmpty();
    }

    const QRegularExpression detailRegex(
        "<h3[^>]*class=\"([^\"]*h3-compile-status[^\"]*)\"[^>]*>([\\s\\S]*?)</h3>"
        "[\\s\\S]*?<pre>([\\s\\S]*?)</pre>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch detailMatch = detailRegex.match(text);
    if (detailMatch.hasMatch()) {
        info.detailTitle = stripHtml(detailMatch.captured(2));
        info.detailText = decodePreformattedText(detailMatch.captured(3));
    }

    return info;
}
}
