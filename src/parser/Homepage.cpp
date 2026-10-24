#include "Homepage.h"

#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/html/interfaces/document.h>

#include <QList>
#include <QUrl>
#include <QStringList>

namespace
{
QString nodeText(lxb_dom_node_t *node)
{
    size_t length = 0;
    const lxb_char_t *text = lxb_dom_node_text_content(node, &length);
    return text == nullptr ? QString() : QString::fromUtf8(
                                             reinterpret_cast<const char *>(text),
                                             static_cast<qsizetype>(length));
}

QString hrefForPersonalHome(lxb_dom_node_t *node, const QUrl &baseUrl)
{
    lxb_dom_element_t *element = lxb_dom_interface_element(node);
    size_t length = 0;
    const lxb_char_t *href = lxb_dom_element_get_attribute(
        element, reinterpret_cast<const lxb_char_t *>("href"), 4, &length);
    if (href == nullptr || length == 0) {
        return QString();
    }

    const QUrl hrefUrl(QString::fromUtf8(reinterpret_cast<const char *>(href),
                                         static_cast<qsizetype>(length)));
    const QUrl url = baseUrl.isEmpty() ? hrefUrl : baseUrl.resolved(hrefUrl);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http")
                           && url.scheme() != QStringLiteral("https"))
        || url.host().compare(QStringLiteral("openjudge.cn"), Qt::CaseInsensitive) != 0) {
        return QString();
    }

    const QStringList pathParts = url.path().split('/', Qt::SkipEmptyParts);
    if (pathParts.size() != 2 || pathParts.at(0) != QStringLiteral("user")) {
        return QString();
    }
    for (const QChar ch : pathParts.at(1)) {
        if (ch < QLatin1Char('0') || ch > QLatin1Char('9')) {
            return QString();
        }
    }
    return url.toString();
}

QString parseHomepageImpl(const QByteArray &html, const QUrl &baseUrl)
{
    lxb_html_document_t *document = lxb_html_document_create();
    if (document == nullptr) {
        return QString();
    }

    const lxb_status_t status = lxb_html_document_parse(
        document, reinterpret_cast<const lxb_char_t *>(html.constData()),
        static_cast<size_t>(html.size()));
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy(document);
        return QString();
    }

    QList<lxb_dom_node_t *> nodes;
    nodes.append(lxb_dom_interface_node(document));
    QString result;
    while (!nodes.isEmpty()) {
        lxb_dom_node_t *node = nodes.takeLast();
        if (node->type == LXB_DOM_NODE_TYPE_ELEMENT
            && lxb_dom_node_tag_id(node) == LXB_TAG_A
            && nodeText(node).simplified() == QStringLiteral("个人首页")) {
            result = hrefForPersonalHome(node, baseUrl);
            if (!result.isEmpty()) {
                break;
            }
        }

        for (lxb_dom_node_t *child = node->first_child; child != nullptr;
             child = child->next) {
            nodes.append(child);
        }
    }

    lxb_html_document_destroy(document);
    return result;
}
}

namespace HomepageParser
{
QString parseHomepage(const QByteArray &html, const QUrl &baseUrl)
{
    return parseHomepageImpl(html, baseUrl);
}
}

