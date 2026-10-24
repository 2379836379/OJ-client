#include "UserHomepage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace UserHomepageParser
{
QList<JoinedClassInfo> parseUserHomepage(const QByteArray &html)
{
    HtmlParserCommon::Document document(html);
    QList<JoinedClassInfo> classes;
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (!HtmlParserCommon::isTag(node, LXB_TAG_A)
            || !HtmlParserCommon::hasClass(node, QStringLiteral("group-name"))) {
            continue;
        }
        const QString name = HtmlParserCommon::text(node).simplified();
        const QString url = HtmlParserCommon::attribute(node, "href");
        if (!name.isEmpty() && !url.isEmpty()) {
            classes.append({name, url});
        }
    }
    return classes;
}
}
