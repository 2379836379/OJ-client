#include "GroupPage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace
{
lxb_dom_node_t *nextElement(lxb_dom_node_t *node)
{
    for (node = node == nullptr ? nullptr : node->next;
         node != nullptr && node->type != LXB_DOM_NODE_TYPE_ELEMENT;
         node = node->next) {
    }
    return node;
}
}

namespace GroupPageParser
{
GroupPageInfo parseGroupPage(const QByteArray &html, const QUrl &baseUrl)
{
    GroupPageInfo info;
    
    HtmlParserCommon::Document document(html);
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (!HtmlParserCommon::isTag(node, LXB_TAG_UL)
            || !HtmlParserCommon::hasClass(node, QStringLiteral("current-contest"))) {
            continue;
        }

        for (lxb_dom_node_t *item = node->first_child; item != nullptr;
             item = item->next) {
            if (!HtmlParserCommon::isTag(item, LXB_TAG_LI)) {
                continue;
            }
            const bool isContest = HtmlParserCommon::hasClass(
                item, QStringLiteral("contest-info"));
            const bool isPractice = HtmlParserCommon::hasClass(
                item, QStringLiteral("practice-info"));
            if (!isContest && !isPractice) {
                continue;
            }

            lxb_dom_node_t *heading = nullptr;
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(item)) {
                if (HtmlParserCommon::isTag(child, LXB_TAG_H3)) {
                    heading = child;
                    break;
                }
            }
            if (heading == nullptr) {
                continue;
            }

            lxb_dom_node_t *link = nullptr;
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(heading)) {
                if (HtmlParserCommon::isTag(child, LXB_TAG_A)) {
                    link = child;
                    break;
                }
            }
            if (link == nullptr) {
                continue;
            }

            ContestSetInfo contestSet;
            contestSet.url = baseUrl.resolved(
                QUrl(HtmlParserCommon::attribute(link, "href"))).toString();
            contestSet.title = HtmlParserCommon::text(link).simplified();
            QString problemCount;
            for (lxb_dom_node_t *sibling = link->next; sibling != nullptr;
                 sibling = sibling->next) {
                if (sibling->type == LXB_DOM_NODE_TYPE_TEXT) {
                    problemCount += HtmlParserCommon::text(sibling);
                }
            }
            QString problemCountDigits;
            for (const QChar character : problemCount) {
                if (character >= QLatin1Char('0')
                    && character <= QLatin1Char('9')) {
                    problemCountDigits.append(character);
                }
            }
            contestSet.problemnumber = problemCountDigits.toInt();

            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(item)) {
                if (!HtmlParserCommon::hasClass(child,
                                                QStringLiteral("over-time"))) {
                    continue;
                }
                const QString value = HtmlParserCommon::text(child).simplified();
                const qsizetype separator = value.indexOf(QChar(0xff1a));
                if (separator >= 0) {
                    contestSet.endtime = value.mid(separator + 1).trimmed();
                }
            }
            info.contestSets.append(contestSet);
        }
        break;
    }
    return info;
}
}
