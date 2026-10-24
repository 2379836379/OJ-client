#include "ContestPage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace
{
lxb_dom_node_t *firstDescendantWithClass(lxb_dom_node_t *root,
                                         lxb_tag_id_t tag,
                                         const QString &className)
{
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(root)) {
        if (HtmlParserCommon::isTag(node, tag)
            && HtmlParserCommon::hasClass(node, className)) {
            return node;
        }
    }
    return nullptr;
}

QString cellText(lxb_dom_node_t *row, const QString &className)
{
    lxb_dom_node_t *cell = firstDescendantWithClass(row, LXB_TAG_TD, className);
    return HtmlParserCommon::text(cell).simplified();
}

QString cellLinkHref(lxb_dom_node_t *row, const QString &className)
{
    lxb_dom_node_t *cell = firstDescendantWithClass(row, LXB_TAG_TD, className);
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(cell)) {
        if (HtmlParserCommon::isTag(node, LXB_TAG_A)) {
            return HtmlParserCommon::attribute(node, "href");
        }
    }
    return QString();
}
}

namespace ContestPageParser
{
ContestPageInfo parseContestPage(const QByteArray &html, const QUrl &baseUrl)
{
    ContestPageInfo info;
    info.contestPageUrl = baseUrl.toString();

    HtmlParserCommon::Document document(html);
    for (lxb_dom_node_t *row : HtmlParserCommon::descendants(document.root())) {
        if (!HtmlParserCommon::isTag(row, LXB_TAG_TR)) {
            continue;
        }
        const QString problemId = cellText(row, QStringLiteral("problem-id"));
        const QString title = cellText(row, QStringLiteral("title"));
        const QString problemIdHref = cellLinkHref(row, QStringLiteral("problem-id"));
        const QString titleHref = cellLinkHref(row, QStringLiteral("title"));
        if (problemId.isEmpty() || title.isEmpty() || problemIdHref.isEmpty()
            || titleHref.isEmpty()) {
            continue;
        }

        ContestProblemInfo problem;
        problem.problemId = problemId;
        problem.title = title;
        problem.problemUrl = baseUrl.resolved(QUrl(titleHref)).toString();

        lxb_dom_node_t *solvedCell = firstDescendantWithClass(
            row, LXB_TAG_TD, QStringLiteral("solved"));
        for (lxb_dom_node_t *node : HtmlParserCommon::descendants(solvedCell)) {
            if (HtmlParserCommon::isTag(node, LXB_TAG_IMG)
                && HtmlParserCommon::attribute(node, "src").contains(
                    QStringLiteral("accepted.gif"), Qt::CaseInsensitive)) {
                problem.solved = true;
                break;
            }
        }

        problem.accept_prople = cellText(row, QStringLiteral("accepted")).toInt();
        problem.submission_people = cellText(row, QStringLiteral("submissions")).toInt();
        info.problems.append(problem);
        ++info.totalProblems;
        if (problem.solved) {
            ++info.solvedProblems;
        }
    }
    return info;
}
}
