#include "submitparser.h"

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

QString extractHiddenValue(const QString &html, const QString &name)
{
    const QString pattern = QString(
        "<input\\s+type=\"hidden\"\\s+name=\"%1\"\\s+value=\"([^\"]*)\"")
                                .arg(QRegularExpression::escape(name));
    const QRegularExpression regex(
        pattern,
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = regex.match(html);
    if (!match.hasMatch()) {
        return QString();
    }

    return match.captured(1).trimmed();
}
}

namespace SubmitParser
{
SubmitPageInfo parseSubmitPage(const QByteArray &html, const QUrl &baseUrl)
{
    SubmitPageInfo info;
    info.pageUrl = baseUrl.toString();

    const QString text = ParserCommon::toUtf8String(html);

    const QRegularExpression formRegex(
        "<form\\s+id=\"solution_submit\"\\s+action=\"([^\"]+)\"[^>]*>",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch formMatch = formRegex.match(text);
    if (formMatch.hasMatch()) {
        info.submitActionUrl =
            baseUrl.resolved(QUrl(formMatch.captured(1))).toString();
    }

    info.contestId = extractHiddenValue(text, "contestId");
    info.problemNumber = extractHiddenValue(text, "problemNumber");
    info.sourceEncode = extractHiddenValue(text, "sourceEncode");

    const QRegularExpression languageRegex(
        "<input\\s+type=\"radio\"\\s+name=\"language\"\\s+value=\"([^\"]+)\"([^>]*)/>\\s*([^<]+)",
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = languageRegex.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        SubmitLanguageOption option;
        option.value = match.captured(1).trimmed();
        option.checked = match.captured(2).contains(
            "checked", Qt::CaseInsensitive);
        option.label = stripHtml(match.captured(3));
        info.languages.append(option);
    }
    return info;
}

QString defaultLanguage(const SubmitPageInfo &pageInfo)
{
    for (const SubmitLanguageOption &option : pageInfo.languages) {
        if (option.checked) {
            return option.value;
        }
    }

    if (!pageInfo.languages.isEmpty()) {
        return pageInfo.languages.first().value;
    }
    return QString();
}

bool hasLanguage(const SubmitPageInfo &pageInfo, const QString &language)
{
    for (const SubmitLanguageOption &option : pageInfo.languages) {
        if (option.value == language) {
            return true;
        }
    }

    return false;
}

QByteArray buildSubmitPayload(const SubmitPageInfo &pageInfo,
                              const QString &language,
                              const QString &sourceText)
{
    const QString selectedLanguage =
        hasLanguage(pageInfo, language) ? language : defaultLanguage(pageInfo);
    const QByteArray encodedSource = sourceText.toUtf8().toBase64();

    const auto encode = [](const QString &value) {
        return QString::fromLatin1(QUrl::toPercentEncoding(value));
    };

    const QString payload = QString(
        "contestId=%1&problemNumber=%2&sourceEncode=%3&language=%4&source=%5")
                                .arg(encode(pageInfo.contestId),
                                     encode(pageInfo.problemNumber),
                                     encode(pageInfo.sourceEncode),
                                     encode(selectedLanguage),
                                     encode(QString::fromUtf8(encodedSource)));

    return payload.toUtf8();
}
}
