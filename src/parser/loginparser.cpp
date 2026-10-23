#include "loginparser.h"

#include <QRegularExpression>
#include <QSet>


namespace LoginParser
{
// 去掉<>和空格
QString stripHtmlTags(const QString &html)
{
    QString text = html;
    text.remove(QRegularExpression("<[^>]+>"));
    return text.simplified();
}

// 个人首页格式：<a href="http://openjudge.cn/user/1111111/">个人首页</a>
QString extractPersonalHomeUrl(const QByteArray &html)
{
    const QString text = QString::fromUtf8(html);
    static const QRegularExpression personalHomeLinkRegex(
        "<a href=\"(http://openjudge\\.cn/user/\\d+/)\">个人首页</a>");
    const QRegularExpressionMatch match = personalHomeLinkRegex.match(text);

    if (!match.hasMatch()) {
        return QString();
    }
    return match.captured(1);
}

// 已加入的小组
//<a class="group-name" href="http://openjudge.cn/user/1481664/in/group-93/">
//<img src="http://static.openjudge.cn/images/default_logo_small.png" width="25" height="25" />NOI</a>
QList<JoinedClassInfo> extractJoinedClasses(const QByteArray &html)
{
    const QString text = QString::fromUtf8(html);
    static const QRegularExpression groupLinkRegex(
    "<a class=\"group-name\" href=\"([^\"]+)\">([\\s\\S]*?)</a>");
    QList<JoinedClassInfo> classes;
    QRegularExpressionMatchIterator it = groupLinkRegex.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString href = match.captured(1);
        const QString innerHtml = match.captured(2);
        classes << JoinedClassInfo{stripHtmlTags(innerHtml), href};
    }
    return classes;
}
}
