#pragma once

#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/html/interfaces/document.h>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace HtmlParserCommon
{
class Document
{
public:
    explicit Document(const QByteArray &html)
        : m_document(lxb_html_document_create())
    {
        if (m_document == nullptr
            || lxb_html_document_parse(
                   m_document,
                   reinterpret_cast<const lxb_char_t *>(html.constData()),
                   static_cast<size_t>(html.size())) != LXB_STATUS_OK) {
            if (m_document != nullptr) {
                lxb_html_document_destroy(m_document);
                m_document = nullptr;
            }
        }
    }

    ~Document()
    {
        if (m_document != nullptr) {
            lxb_html_document_destroy(m_document);
        }
    }

    Document(const Document &) = delete;
    Document &operator=(const Document &) = delete;

    lxb_dom_node_t *root() const
    {
        return m_document == nullptr ? nullptr
                                     : lxb_dom_interface_node(m_document);
    }

private:
    lxb_html_document_t *m_document = nullptr;
};

inline QList<lxb_dom_node_t *> descendants(lxb_dom_node_t *root)
{
    QList<lxb_dom_node_t *> result;
    if (root == nullptr) {
        return result;
    }

    QList<lxb_dom_node_t *> pending{root};
    while (!pending.isEmpty()) {
        lxb_dom_node_t *node = pending.takeLast();
        result.append(node);
        QList<lxb_dom_node_t *> children;
        for (lxb_dom_node_t *child = node->first_child; child != nullptr;
             child = child->next) {
            children.append(child);
        }
        for (auto it = children.crbegin(); it != children.crend(); ++it) {
            pending.append(*it);
        }
    }
    return result;
}

inline QString text(lxb_dom_node_t *node)
{
    if (node == nullptr) {
        return QString();
    }
    size_t length = 0;
    const lxb_char_t *value = lxb_dom_node_text_content(node, &length);
    return value == nullptr
               ? QString()
               : QString::fromUtf8(reinterpret_cast<const char *>(value),
                                   static_cast<qsizetype>(length));
}

inline QString attribute(lxb_dom_node_t *node, const char *name)
{
    if (node == nullptr || node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return QString();
    }
    const size_t nameLength = qstrlen(name);
    size_t valueLength = 0;
    const lxb_char_t *value = lxb_dom_element_get_attribute(
        lxb_dom_interface_element(node),
        reinterpret_cast<const lxb_char_t *>(name), nameLength, &valueLength);
    return value == nullptr
               ? QString()
               : QString::fromUtf8(reinterpret_cast<const char *>(value),
                                   static_cast<qsizetype>(valueLength));
}

inline bool hasAttribute(lxb_dom_node_t *node, const char *name)
{
    if (node == nullptr || node->type != LXB_DOM_NODE_TYPE_ELEMENT) {
        return false;
    }
    return lxb_dom_element_has_attribute(
        lxb_dom_interface_element(node),
        reinterpret_cast<const lxb_char_t *>(name), qstrlen(name));
}

inline bool hasClass(lxb_dom_node_t *node, const QString &className)
{
    return attribute(node, "class").split(QChar::Space, Qt::SkipEmptyParts)
        .contains(className);
}

inline bool isTag(lxb_dom_node_t *node, lxb_tag_id_t tag)
{
    return node != nullptr && node->type == LXB_DOM_NODE_TYPE_ELEMENT
           && lxb_dom_node_tag_id(node) == tag;
}

inline QStringList textLines(lxb_dom_node_t *node)
{
    QString value = text(node);
    value.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    value.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return value.split(QLatin1Char('\n'));
}
}
