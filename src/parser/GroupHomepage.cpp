#include "GroupHomepage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace GroupHomepageParser
{
QString parseGroupHomepage(const QByteArray &html)
{
    HtmlParserCommon::Document document(html);
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_A)
            && HtmlParserCommon::text(node).simplified()
                   == QStringLiteral("前往小组")) {
            return HtmlParserCommon::attribute(node, "href");
        }
    }
    return QString();
}
}
