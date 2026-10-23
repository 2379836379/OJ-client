#include "HomepageParser.h"

#include <QRegularExpression>
#include <QSet>


namespace HomepageParser
{
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
}
