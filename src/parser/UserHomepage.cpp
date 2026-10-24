#include "UserHomepage.h"

#include <QRegularExpression>



namespace UserHomepageParser
{
QString stripHtmlTags(const QString &html)
{
    QString text = html;
    text.remove(QRegularExpression("<[^>]+>"));
    return text.simplified();
}
// 已加入的小组
//<a class="group-name" href="http://openjudge.cn/user/1481664/in/group-93/">
//<img src="http://static.openjudge.cn/images/default_logo_small.png" width="25" height="25" />NOI</a>
QList<JoinedClassInfo> parseUserHomepage(const QByteArray &html)
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
