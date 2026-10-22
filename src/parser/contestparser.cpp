#include "contestparser.h"

#include "parsercommon.h"

#include <QRegularExpression>
// 题目列表
namespace ContestParser
{
ContestPageInfo parseContestPage(const QByteArray &html, const QUrl &baseUrl)
{
    ContestPageInfo info;
    info.contestPageUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);
    const QRegularExpression rowRegex(
        "<tr(?:\\s+class=\"alt\")?>([\\s\\S]*?)</tr>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression solvedRegex(
        "<td\\s+class=\"solved\"[^>]*>[\\s\\S]*?accepted\\.gif",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression idRegex(
        "<td\\s+class=\"problem-id\"[^>]*>\\s*<a\\s+href=\"([^\"]+)\">([^<]+)</a>\\s*</td>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression titleRegex(
        "<td\\s+class=\"title\"[^>]*>\\s*<a\\s+href=\"([^\"]+)\">([^<]+)</a>\\s*</td>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression acceptedRegex(
        "<td\\s+class=\"accepted\"[^>]*>[\\s\\S]*?>(\\d+)</a>\\s*</td>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression submissionsRegex(
        "<td\\s+class=\"submissions\"[^>]*>[\\s\\S]*?>(\\d+)</a>\\s*</td>",
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator rowIt = rowRegex.globalMatch(text);
    while (rowIt.hasNext()) {
        const QRegularExpressionMatch rowMatch = rowIt.next();
        const QString rowHtml = rowMatch.captured(1);

        const QRegularExpressionMatch idMatch = idRegex.match(rowHtml);
        const QRegularExpressionMatch titleMatch = titleRegex.match(rowHtml);
        if (!idMatch.hasMatch() || !titleMatch.hasMatch()) {
            continue;
        }
        ContestProblemInfo problem;
        problem.solved = solvedRegex.match(rowHtml).hasMatch();
        problem.problemId = idMatch.captured(2).trimmed();
        problem.problemUrl = baseUrl.resolved(QUrl(titleMatch.captured(1))).toString();
        problem.title = titleMatch.captured(2).trimmed();

        const QRegularExpressionMatch acceptedMatch =
            acceptedRegex.match(rowHtml);
        if (acceptedMatch.hasMatch()) {
            problem.accept_prople = acceptedMatch.captured(1).toInt();
        }

        const QRegularExpressionMatch submissionsMatch =
            submissionsRegex.match(rowHtml);
        if (submissionsMatch.hasMatch()) {
            problem.submission_people = submissionsMatch.captured(1).toInt();
        }
        info.problems.append(problem);
        ++info.totalProblems;
        if (problem.solved) {
            ++info.solvedProblems;
        }
    }
    return info;
}
}
