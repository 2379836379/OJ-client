#include "problemparser.h"

#include "parsercommon.h"

#include <QRegularExpression>

namespace
{
QString stripHtml(const QString &html)
{
    QString text = html;
    text.remove(QRegularExpression("<[^>]+>"));
    return text.simplified();
}

QString extractDlValue(const QString &html, const QString &key)
{
    const QString pattern = QString(
        "<dt>\\s*%1\\s*</dt>\\s*<dd>([\\s\\S]*?)</dd>")
                                .arg(QRegularExpression::escape(key));
    const QRegularExpression regex(
        pattern,
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = regex.match(html);
    if (!match.hasMatch()) {
        return QString();
    }

    return stripHtml(match.captured(1));
}

QString extractDlValueAny(const QString &html, const QStringList &keys)
{
    for (const QString &key : keys) {
        const QString value = extractDlValue(html, key);
        if (!value.isEmpty()) {
            return value;
        }
    }
    return QString();
}

QString extractSectionHtml(const QString &html, const QString &key)
{
    const QString pattern = QString(
        "<dt>\\s*%1\\s*</dt>([\\s\\S]*?)(?=<dt>|</dl>)")
                                .arg(QRegularExpression::escape(key));
    const QRegularExpression regex(
        pattern,
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = regex.match(html);
    return match.hasMatch() ? match.captured(1) : QString();
}

QString extractPreBlocksText(const QString &html)
{
    QStringList blocks;
    const QRegularExpression preRegex(
        "<pre[^>]*>([\\s\\S]*?)</pre>",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = preRegex.globalMatch(html);
    while (it.hasNext()) {
        QString block = it.next().captured(1);
        block.replace("\r\n", "\n");
        block.replace('\r', '\n');
        block.replace("&lt;", "<");
        block.replace("&gt;", ">");
        block.replace("&amp;", "&");
        if (!block.trimmed().isEmpty()) {
            blocks << block.trimmed();
        }
    }
    return blocks.join("\n\n");
}

QString extractSectionHtmlAny(const QString &html, const QStringList &keys)
{
    for (const QString &key : keys) {
        const QString section = extractSectionHtml(html, key);
        if (!section.isEmpty()) {
            return section;
        }
    }
    return QString();
}
}

namespace ProblemParser
{
ProblemPageInfo parseProblemPage(const QByteArray &html, const QUrl &baseUrl)
{
    ProblemPageInfo info;
    info.problemUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);

    const QRegularExpression titleRegex(
        "<div\\s+id=\"pageTitle\"[^>]*>\\s*<h2>([^<]+)</h2>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch titleMatch = titleRegex.match(text);
    if (titleMatch.hasMatch()) {
        info.title = titleMatch.captured(1).trimmed();
    }

    const QRegularExpression submitRegex(
        "<a\\s+href=\"([^\"]+/submit/)\"[^>]*>\\s*(?:提交|鎻愪氦)\\s*</a>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch submitMatch = submitRegex.match(text);
    if (submitMatch.hasMatch()) {
        info.submitUrl =
            baseUrl.resolved(QUrl(submitMatch.captured(1))).toString();
    }

    info.timeLimit = extractDlValueAny(text, {"总时间限制:", "总时间限制", "鎬绘椂闂撮檺鍒?", "鎬绘椂闂撮檺鍒?"});
    info.memoryLimit = extractDlValueAny(text, {"内存限制:", "内存限制", "鍐呭瓨闄愬埗:", "鍐呭瓨闄愬埗"});
    info.description = extractDlValueAny(text, {"描述", "鎻忚堪"});
    info.starterCode = extractPreBlocksText(extractSectionHtmlAny(text, {"描述", "鎻忚堪"}));
    info.inputSpec = extractDlValueAny(text, {"输入", "杈撳叆"});
    info.outputSpec = extractDlValueAny(text, {"输出", "杈撳嚭"});
    info.sampleInput = extractDlValueAny(text, {"样例输入", "鏍蜂緥杈撳叆"});
    info.sampleOutput = extractDlValueAny(text, {"样例输出", "鏍蜂緥杈撳嚭"});
    info.hint = extractDlValueAny(text, {"提示", "鎻愮ず"});
    info.tried_people = extractDlValueAny(text, {"尝试人数", "灏濊瘯浜烘暟"}).toInt();
    info.passed_people = extractDlValueAny(text, {"通过人数", "閫氳繃浜烘暟"}).toInt();
    return info;
}
}
