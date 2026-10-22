#include "loginparser.h"

#include "parsercommon.h"

#include <QRegularExpression>
#include <QSet>


namespace LoginParser
{
namespace
{
QString stripHtmlTags(const QString &html)
{
    QString text = html;
    text.remove(QRegularExpression("<[^>]+>"));
    return text.simplified();
}
}

QString extractPersonalHomeUrl(const QByteArray &html, const QUrl &baseUrl)
{
    const QString text = ParserCommon::toUtf8String(html);
    const QRegularExpression personalHomeLinkRegex(
        "<a\\s+href=\"([^\"]*/user/\\d+/?)\"[^>]*>");
    const QRegularExpressionMatch match = personalHomeLinkRegex.match(text);

    if (!match.hasMatch()) {
        return QString();
    }
    return baseUrl.resolved(QUrl(match.captured(1))).toString();
}

QList<JoinedClassInfo> extractJoinedClasses(const QByteArray &html,
                                            const QUrl &baseUrl)
{
    const QString text = ParserCommon::toUtf8String(html);
    const QRegularExpression groupLinkRegex(
        "<a\\s+[^>]*class=\"[^\"]*\\bgroup-name\\b[^\"]*\"[^>]*href=\"([^\"]+)\""
        "[^>]*>([\\s\\S]*?)</a>");

    QList<JoinedClassInfo> classes;
    QRegularExpressionMatchIterator it = groupLinkRegex.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString href = match.captured(1);
        const QString innerHtml = match.captured(2);
        const QString url = baseUrl.resolved(QUrl(href)).toString();
        classes << JoinedClassInfo{stripHtmlTags(innerHtml), url};
    }
    return classes;
}
}
