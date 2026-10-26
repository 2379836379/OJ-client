#include "ProblemPage.h"

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

QString normalizeLabel(const QString &label)
{
    QString normalized = label.simplified();
    if (normalized.endsWith(QLatin1Char(':'))
        || normalized.endsWith(QChar(0xff1a))) {
        normalized.chop(1);
    }
    return normalized;
}

lxb_dom_node_t *definitionValue(lxb_dom_node_t *list,
                                const QStringList &labels)
{
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(list)) {
        if (!HtmlParserCommon::isTag(node, LXB_TAG_DT)
            || !labels.contains(normalizeLabel(HtmlParserCommon::text(node)))) {
            continue;
        }
        lxb_dom_node_t *value = nextElementSibling(node);
        return HtmlParserCommon::isTag(value, LXB_TAG_DD) ? value : nullptr;
    }
    return nullptr;
}

QString valueText(lxb_dom_node_t *list, const QStringList &labels)
{
    return HtmlParserCommon::text(definitionValue(list, labels)).simplified();
}

QString preformattedText(lxb_dom_node_t *node)
{
    QString value = HtmlParserCommon::text(node);
    value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return value.trimmed();
}
}

namespace ProblemPageParser
{
ProblemPageInfo parseProblemPage(const QByteArray &html, const QUrl &baseUrl)
{
    ProblemPageInfo info;
    info.problemUrl = baseUrl.toString();

    HtmlParserCommon::Document document(html);
    lxb_dom_node_t *problemContent = nullptr;
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_DIV)
            && HtmlParserCommon::attribute(node, "id") == QStringLiteral("pageTitle")) {
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(node)) {
                if (HtmlParserCommon::isTag(child, LXB_TAG_H2)) {
                    info.title = HtmlParserCommon::text(child).simplified();
                    break;
                }
            }
        }
        if (HtmlParserCommon::isTag(node, LXB_TAG_DL)
                   && HtmlParserCommon::hasClass(
                       node, QStringLiteral("problem-content"))) {
            problemContent = node;
        }
    }
    
    info.submitUrl = baseUrl.resolved(QUrl("submit/")).toString();

    lxb_dom_node_t *params = nullptr;
    lxb_dom_node_t *statistics = nullptr;
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_DL)
            && HtmlParserCommon::hasClass(node, QStringLiteral("problem-params"))) {
            params = node;
        }
        if (HtmlParserCommon::isTag(node, LXB_TAG_DIV)
            && HtmlParserCommon::hasClass(
                node, QStringLiteral("problem-statistics"))) {
            for (lxb_dom_node_t *child : HtmlParserCommon::descendants(node)) {
                if (HtmlParserCommon::isTag(child, LXB_TAG_DL)) {
                    statistics = child;
                    break;
                }
            }
        }
    }

    info.timeLimit = valueText(params, {QStringLiteral("总时间限制")});
    info.memoryLimit = valueText(params, {QStringLiteral("内存限制")});
    info.description = valueText(problemContent, {QStringLiteral("描述")});
    info.inputSpec = valueText(problemContent, {QStringLiteral("输入")});
    info.outputSpec = valueText(problemContent, {QStringLiteral("输出")});
    info.sampleInput = valueText(problemContent, {QStringLiteral("样例输入")});
    info.sampleOutput = valueText(problemContent, {QStringLiteral("样例输出")});
    info.hint = valueText(problemContent, {QStringLiteral("提示")});

    lxb_dom_node_t *description = definitionValue(
        problemContent, {QStringLiteral("描述")});
    QStringList blocks;
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(description)) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_PRE)) {
            const QString block = preformattedText(node);
            if (!block.isEmpty()) {
                blocks.append(block);
            }
        }
    }
    info.starterCode = blocks.join(QStringLiteral("\n\n"));
    return info;
}
}
