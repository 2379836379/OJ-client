#include "ResultPage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace
{
lxb_dom_node_t *nextElementSibling(lxb_dom_node_t *node)
{
    for (node = node == nullptr ? nullptr : node->next;
         node != nullptr && node->type != LXB_DOM_NODE_TYPE_ELEMENT;
         node = node->next) {
    }
    return node;
}
}

namespace ResultPageParser
{
ResultPageInfo parseResultPage(const QByteArray &html, const QUrl &baseUrl)
{
    ResultPageInfo info;
    info.pageUrl = baseUrl.toString();

    HtmlParserCommon::Document document(html);
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_DIV)
            && HtmlParserCommon::attribute(node, "id")
                   == QStringLiteral("pageTitle")) {
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(node)) {
                if (!HtmlParserCommon::isTag(child, LXB_TAG_H2)) {
                    continue;
                }
                QString title = HtmlParserCommon::text(child).simplified();
                if (title.startsWith(QLatin1Char('#'))) {
                    title.remove(0, 1);
                }
                const qsizetype suffix = title.indexOf(QStringLiteral("提交状态"));
                if (suffix >= 0) {
                    info.submissionId = title.left(suffix).trimmed();
                }
                break;
            }
        }

        if (HtmlParserCommon::isTag(node, LXB_TAG_P)
            && HtmlParserCommon::hasClass(node, QStringLiteral("compile-status"))) {
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(node)) {
                if (!HtmlParserCommon::isTag(child, LXB_TAG_A)) {
                    continue;
                }
                info.solutionUrl = baseUrl.resolved(
                    QUrl(HtmlParserCommon::attribute(child, "href"))).toString();
                info.statusClass = HtmlParserCommon::attribute(child, "class");
                info.statusText = HtmlParserCommon::text(child).simplified();
                info.hasResult = !info.statusText.isEmpty();
                break;
            }
        }

        if (HtmlParserCommon::isTag(node, LXB_TAG_H3)
            && HtmlParserCommon::hasClass(
                node, QStringLiteral("h3-compile-status"))) {
            info.detailTitle = HtmlParserCommon::text(node).simplified();
            lxb_dom_node_t *following = nextElementSibling(node);
            for (lxb_dom_node_t *candidate : HtmlParserCommon::descendants(following)) {
                if (HtmlParserCommon::isTag(candidate, LXB_TAG_PRE)) {
                    info.detailText = HtmlParserCommon::text(candidate).trimmed();
                    break;
                }
            }
        }
    }
    return info;
}
}
