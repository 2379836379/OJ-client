#include "SubmitPage.h"

#include "HtmlParserCommon.h"

#include <lexbor/tag/tag.h>

namespace SubmitPageParser
{
SubmitPageInfo parseSubmitPage(const QByteArray &html, const QUrl &baseUrl)
{
    SubmitPageInfo info;
    info.pageUrl = baseUrl.toString();

    HtmlParserCommon::Document document(html);
    for (lxb_dom_node_t *node : HtmlParserCommon::descendants(document.root())) {
        if (!HtmlParserCommon::isTag(node, LXB_TAG_FORM)
            || HtmlParserCommon::attribute(node, "id")
                   != QStringLiteral("solution_submit")) {
            continue;
        }
        info.submitActionUrl = baseUrl.resolved(
            QUrl(HtmlParserCommon::attribute(node, "action"))).toString();

        for (lxb_dom_node_t *field : HtmlParserCommon::descendants(node)) {
            if (!HtmlParserCommon::isTag(field, LXB_TAG_INPUT)) {
                continue;
            }
            const QString name = HtmlParserCommon::attribute(field, "name");
            const QString type = HtmlParserCommon::attribute(field, "type");
            if (type.compare(QStringLiteral("hidden"), Qt::CaseInsensitive) == 0) {
                const QString value = HtmlParserCommon::attribute(field, "value").trimmed();
                if (name == QStringLiteral("contestId")) {
                    info.contestId = value;
                } else if (name == QStringLiteral("problemNumber")) {
                    info.problemNumber = value;
                } else if (name == QStringLiteral("sourceEncode")) {
                    info.sourceEncode = value;
                }
            } else if (type.compare(QStringLiteral("radio"), Qt::CaseInsensitive) == 0
                       && name == QStringLiteral("language")) {
                SubmitLanguageOption option;
                option.value = HtmlParserCommon::attribute(field, "value").trimmed();
                option.checked = HtmlParserCommon::hasAttribute(field, "checked");
                for (lxb_dom_node_t *sibling = field->next; sibling != nullptr;
                     sibling = sibling->next) {
                    if (sibling->type == LXB_DOM_NODE_TYPE_TEXT) {
                        option.label += HtmlParserCommon::text(sibling);
                    } else if (sibling->type == LXB_DOM_NODE_TYPE_ELEMENT
                               && HtmlParserCommon::isTag(sibling, LXB_TAG_BR)) {
                        break;
                    } else if (sibling->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                        break;
                    }
                }
                option.label = option.label.simplified();
                if (!option.value.isEmpty()) {
                    info.languages.append(option);
                }
            }
        }
        break;
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

    return pageInfo.languages.isEmpty() ? QString() : pageInfo.languages.first().value;
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
