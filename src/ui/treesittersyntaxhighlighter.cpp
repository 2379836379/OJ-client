#include "ui/treesittersyntaxhighlighter.h"

#include "config/apppaths.h"

#include <tree_sitter/api.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTimer>
#include <QVector>

namespace
{
extern "C" const TSLanguage *tree_sitter_cpp(void);
extern "C" const TSLanguage *tree_sitter_python(void);

constexpr int kMaxTreeSitterBytes = 512 * 1024;

QString resolveQueryPath(const QString &relativePath)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    const QStringList candidates = {
        QDir(cwd).filePath(relativePath),
        QDir(appDir).filePath(relativePath),
        QDir(appDir).filePath("../" + relativePath),
        QDir(appDir).filePath("../oj-client/" + relativePath),
        QDir(appDir).filePath("../../oj-client/" + relativePath),
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return QDir::cleanPath(path);
        }
    }
    return QString();
}

QByteArray loadQuerySource(const QString &relativePath)
{
    const QString path = resolveQueryPath(relativePath);
    if (path.isEmpty()) {
        return {};
    }

    QFile queryFile(path);
    if (!queryFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return queryFile.readAll();
}

void appendHighlighterLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | TreeSitterSyntaxHighlighter: " << message << '\n';
}

TSNode nodeFromPtr(void *nodePtr)
{
    return *static_cast<TSNode *>(nodePtr);
}

QByteArray nodeType(TSNode node)
{
    return QByteArray(ts_node_type(node));
}

bool nodeTypeIs(TSNode node, const char *typeName)
{
    return ts_node_type(node) != nullptr
           && QByteArray(ts_node_type(node)) == QByteArray(typeName);
}

TSNode firstNamedChildOfType(TSNode node, const char *typeName)
{
    const uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        if (!ts_node_is_named(child)) {
            continue;
        }
        if (nodeTypeIs(child, typeName)) {
            return child;
        }
    }
    return TSNode{};
}

TSNode fieldChild(TSNode node, const char *fieldName)
{
    return ts_node_child_by_field_name(node, fieldName, static_cast<uint32_t>(qstrlen(fieldName)));
}
}

TreeSitterSyntaxHighlighter::TreeSitterSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_parser = ts_parser_new();
    m_queryCursor = ts_query_cursor_new();
    connect(document(), &QTextDocument::contentsChanged, this, [this]() {
        scheduleParse();
    });
}

TreeSitterSyntaxHighlighter::~TreeSitterSyntaxHighlighter()
{
    clearTree();
    clearQuery();
    if (m_queryCursor != nullptr) {
        ts_query_cursor_delete(m_queryCursor);
        m_queryCursor = nullptr;
    }
    if (m_parser != nullptr) {
        ts_parser_delete(m_parser);
        m_parser = nullptr;
    }
}

void TreeSitterSyntaxHighlighter::setLanguageMode(LanguageMode mode)
{
    if (m_languageMode == mode) {
        return;
    }

    m_languageMode = mode;
    const QuerySpec spec = buildQuerySpec(mode);
    if (m_parser != nullptr) {
        ts_parser_set_language(m_parser, spec.language);
    }
    reloadQuery();
    scheduleParse();
}

void TreeSitterSyntaxHighlighter::setDarkMode(bool dark)
{
    if (m_darkMode == dark) {
        return;
    }
    m_darkMode = dark;
    rehighlight();
}

void TreeSitterSyntaxHighlighter::scheduleParse()
{
    if (m_parseScheduled || m_parsing) {
        return;
    }

    m_parseScheduled = true;
    QTimer::singleShot(0, this, [this]() {
        m_parseScheduled = false;
        if (m_parsing) {
            return;
        }
        m_parsing = true;
        parseDocument();
        m_parsing = false;
        rehighlight();
    });
}

void TreeSitterSyntaxHighlighter::highlightBlock(const QString &text)
{
    Q_UNUSED(text);

    const int blockNumber = currentBlock().blockNumber();
    if (blockNumber < 0 || blockNumber >= m_blockRanges.size()) {
        return;
    }

    for (const HighlightSpan &span : m_blockRanges.at(blockNumber)) {
        QTextCharFormat format;
        applyFormat(span.kind, &format);
        if (span.length > 0) {
            setFormat(span.start, span.length, format);
        }
    }
}

TreeSitterSyntaxHighlighter::QuerySpec TreeSitterSyntaxHighlighter::buildQuerySpec(
    LanguageMode mode) const
{
    switch (mode) {
    case LanguageMode::Cpp:
        return {tree_sitter_cpp(),
                QStringLiteral("third_party/tree-sitter-cpp/queries/highlights.scm")};
    case LanguageMode::Python:
        return {tree_sitter_python(),
                QStringLiteral("third_party/tree-sitter-python/queries/highlights.scm")};
    case LanguageMode::PlainText:
    default:
        return {};
    }
}

void TreeSitterSyntaxHighlighter::reloadQuery()
{
    clearQuery();

    const QuerySpec spec = buildQuerySpec(m_languageMode);
    if (spec.language == nullptr || spec.queryPath.isEmpty()) {
        return;
    }

    QByteArray querySource;
    if (m_languageMode == LanguageMode::Cpp) {
        querySource =
            loadQuerySource(QStringLiteral("third_party/tree-sitter-cpp/queries/highlights.scm"));
        querySource += R"QUERY(

; Supplemental C/C++ coverage

(comment) @comment

[
  (string_literal)
  (raw_string_literal)
  (char_literal)
] @string

(escape_sequence) @escape

[
  (number_literal)
] @number

[
  (true)
  (false)
] @constant

(null "nullptr" @constant)

[
  "const"
  "break"
  "case"
  "continue"
  "default"
  "do"
  "else"
  "enum"
  "extern"
  "for"
  "goto"
  "if"
  "inline"
  "operator"
  "return"
  "sizeof"
  "static"
  "struct"
  "switch"
  "typedef"
  "union"
  "volatile"
  "while"
] @keyword

)QUERY";
    } else {
        querySource = loadQuerySource(spec.queryPath);
    }
    if (querySource.isEmpty()) {
        return;
    }

    auto tryBuildQuery = [spec](const QByteArray &source) -> TSQuery * {
        uint32_t errorOffset = 0;
        TSQueryError errorType = TSQueryErrorNone;
        TSQuery *query = ts_query_new(spec.language,
                                      source.constData(),
                                      static_cast<uint32_t>(source.size()),
                                      &errorOffset,
                                      &errorType);
        if (errorType != TSQueryErrorNone) {
            appendHighlighterLog(
                QString("query compile failed, errorType=%1, errorOffset=%2")
                    .arg(static_cast<int>(errorType))
                    .arg(errorOffset));
            if (query != nullptr) {
                ts_query_delete(query);
            }
            return nullptr;
        }
        return query;
    };

    m_query = tryBuildQuery(querySource);
    if (m_query != nullptr && m_languageMode == LanguageMode::Cpp) {
        appendHighlighterLog("cpp enhanced query compiled successfully");
    }
    if (m_query == nullptr && m_languageMode == LanguageMode::Cpp) {
        const QByteArray baseQuery =
            loadQuerySource(QStringLiteral("third_party/tree-sitter-cpp/queries/highlights.scm"));
        if (!baseQuery.isEmpty()) {
            m_query = tryBuildQuery(baseQuery);
            appendHighlighterLog(m_query == nullptr
                                     ? "cpp base query fallback failed"
                                     : "cpp base query fallback compiled successfully");
        }
    }
}

void TreeSitterSyntaxHighlighter::parseDocument()
{
    m_blockRanges.clear();
    if (document() == nullptr) {
        return;
    }

    m_blockRanges.resize(document()->blockCount());
    if (m_languageMode == LanguageMode::PlainText || m_parser == nullptr) {
        clearTree();
        return;
    }

    const QString text = document()->toPlainText();
    const QByteArray utf8 = text.toUtf8();
    if (utf8.size() > kMaxTreeSitterBytes) {
        clearTree();
        return;
    }

    QVector<int> byteToUtf16(utf8.size() + 1, 0);
    int byteOffset = 0;
    int utf16Pos = 0;
    for (const QChar ch : text) {
        const QByteArray encoded = QString(ch).toUtf8();
        for (int i = 0; i < encoded.size(); ++i) {
            byteToUtf16[byteOffset + i] = utf16Pos;
        }
        byteOffset += encoded.size();
        ++utf16Pos;
        byteToUtf16[byteOffset] = utf16Pos;
    }

    clearTree();
    m_tree = ts_parser_parse_string(
        m_parser,
        nullptr,
        utf8.constData(),
        static_cast<uint32_t>(utf8.size()));
    if (m_tree == nullptr) {
        return;
    }

    addQueryCaptures(byteToUtf16);
    if (m_languageMode == LanguageMode::Cpp) {
        addCppSemanticSpans(utf8, byteToUtf16);
    }
}

void TreeSitterSyntaxHighlighter::addCppSemanticSpans(const QByteArray &utf8,
                                                      const QVector<int> &byteToUtf16)
{
    if (m_tree == nullptr) {
        return;
    }

    TSNode root = ts_tree_root_node(m_tree);
    collectCppSemanticNode(&root, utf8, byteToUtf16);
}

void TreeSitterSyntaxHighlighter::collectCppSemanticNode(void *nodePtr,
                                                         const QByteArray &utf8,
                                                         const QVector<int> &byteToUtf16)
{
    TSNode node = nodeFromPtr(nodePtr);
    if (ts_node_is_null(node)) {
        return;
    }

    const QByteArray type = nodeType(node);
    auto addNodeSpan = [this, &byteToUtf16](TSNode targetNode, SpanKind kind) {
        if (ts_node_is_null(targetNode)) {
            return;
        }
        const uint32_t startByte = ts_node_start_byte(targetNode);
        const uint32_t endByte = ts_node_end_byte(targetNode);
        if (startByte >= endByte || endByte >= static_cast<uint32_t>(byteToUtf16.size())) {
            return;
        }
        const int startPos = byteToUtf16.at(static_cast<int>(startByte));
        const int endPos = byteToUtf16.at(static_cast<int>(endByte));
        if (endPos > startPos) {
            addSpan(startPos, endPos - startPos, kind);
        }
    };

    if (type == "namespace_identifier") {
        addNodeSpan(node, SpanKind::Namespace);
    } else if (type == "type_identifier"
               || type == "primitive_type"
               || type == "sized_type_specifier") {
        addNodeSpan(node, SpanKind::Type);
    } else if (type == "field_identifier") {
        addNodeSpan(node, SpanKind::Property);
    } else if (type == "system_lib_string") {
        addNodeSpan(node, SpanKind::IncludePath);
    } else if (type == "preproc_include"
               || type == "preproc_def"
               || type == "preproc_function_def"
               || type == "preproc_if"
               || type == "preproc_ifdef"
               || type == "preproc_else"
               || type == "preproc_elif"
               || type == "preproc_ifndef"
               || type == "preproc_call") {
        addNodeSpan(node, SpanKind::Preprocessor);
        if (type == "preproc_include") {
            const TSNode pathNode = firstNamedChildOfType(node, "string_literal");
            addNodeSpan(pathNode, SpanKind::IncludePath);
            const TSNode systemPathNode = firstNamedChildOfType(node, "system_lib_string");
            addNodeSpan(systemPathNode, SpanKind::IncludePath);
        } else {
            const TSNode nameNode = fieldChild(node, "name");
            addNodeSpan(nameNode, SpanKind::Macro);
        }
    } else if (type == "call_expression") {
        const TSNode functionNode = fieldChild(node, "function");
        if (!ts_node_is_null(functionNode)) {
            if (nodeTypeIs(functionNode, "identifier")
                || nodeTypeIs(functionNode, "field_identifier")) {
                addNodeSpan(functionNode, SpanKind::Function);
            } else if (nodeTypeIs(functionNode, "qualified_identifier")) {
                TSNode nameNode = fieldChild(functionNode, "name");
                if (ts_node_is_null(nameNode)) {
                    nameNode = firstNamedChildOfType(functionNode, "identifier");
                }
                addNodeSpan(nameNode, SpanKind::Function);
            }
        }
    } else if (type == "qualified_identifier"
               || type == "qualified_type_identifier") {
        const TSNode scopeNode = fieldChild(node, "scope");
        const TSNode nameNode = fieldChild(node, "name");
        if (nodeTypeIs(scopeNode, "namespace_identifier")) {
            addNodeSpan(scopeNode, SpanKind::Namespace);
        } else if (nodeTypeIs(scopeNode, "type_identifier")) {
            addNodeSpan(scopeNode, SpanKind::Type);
        }
        if (nodeTypeIs(nameNode, "identifier")) {
            addNodeSpan(nameNode, SpanKind::Function);
        } else if (nodeTypeIs(nameNode, "field_identifier")) {
            addNodeSpan(nameNode, SpanKind::Property);
        } else if (nodeTypeIs(nameNode, "type_identifier")) {
            addNodeSpan(nameNode, SpanKind::Type);
        } else if (nodeTypeIs(nameNode, "namespace_identifier")) {
            addNodeSpan(nameNode, SpanKind::Namespace);
        }
    } else if (type == "this") {
        addNodeSpan(node, SpanKind::Builtin);
    } else if (type == "identifier") {
        const uint32_t startByte = ts_node_start_byte(node);
        const uint32_t endByte = ts_node_end_byte(node);
        if (startByte < endByte && endByte <= static_cast<uint32_t>(utf8.size())) {
            const QByteArray text = utf8.mid(static_cast<int>(startByte),
                                             static_cast<int>(endByte - startByte));
            if (text == "std") {
                addNodeSpan(node, SpanKind::Namespace);
            } else {
                bool looksLikeConstant = !text.isEmpty();
                bool hasLetter = false;
                for (const char ch : text) {
                    if (ch >= 'a' && ch <= 'z') {
                        looksLikeConstant = false;
                        break;
                    }
                    if (ch >= 'A' && ch <= 'Z') {
                        hasLetter = true;
                    } else if (!((ch >= '0' && ch <= '9') || ch == '_')) {
                        looksLikeConstant = false;
                        break;
                    }
                }
                if (looksLikeConstant && hasLetter) {
                    addNodeSpan(node, SpanKind::Constant);
                }
            }
        }
    }

    const uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        collectCppSemanticNode(&child, utf8, byteToUtf16);
    }
}

void TreeSitterSyntaxHighlighter::addQueryCaptures(const QVector<int> &byteToUtf16)
{
    if (m_query == nullptr || m_queryCursor == nullptr || m_tree == nullptr) {
        return;
    }

    TSNode root = ts_tree_root_node(m_tree);
    ts_query_cursor_exec(m_queryCursor, m_query, root);

    TSQueryMatch match;
    uint32_t captureIndex = 0;
    while (ts_query_cursor_next_capture(m_queryCursor, &match, &captureIndex)) {
        if (captureIndex >= match.capture_count) {
            continue;
        }

        const TSQueryCapture capture = match.captures[captureIndex];
        uint32_t captureNameLength = 0;
        const char *captureNameChars =
            ts_query_capture_name_for_id(m_query, capture.index, &captureNameLength);
        const QByteArray captureName(captureNameChars, static_cast<int>(captureNameLength));
        const SpanKind kind = mapCaptureName(captureName);

        const uint32_t startByte = ts_node_start_byte(capture.node);
        const uint32_t endByte = ts_node_end_byte(capture.node);
        if (startByte >= endByte || endByte >= static_cast<uint32_t>(byteToUtf16.size())) {
            continue;
        }

        const int startPos = byteToUtf16.at(static_cast<int>(startByte));
        const int endPos = byteToUtf16.at(static_cast<int>(endByte));
        if (endPos <= startPos) {
            continue;
        }

        addSpan(startPos, endPos - startPos, kind);
    }

    ts_query_cursor_set_byte_range(m_queryCursor, 0, UINT32_MAX);
}

void TreeSitterSyntaxHighlighter::addSpan(int start, int length, SpanKind kind)
{
    QTextBlock startBlock = document()->findBlock(start);
    QTextBlock endBlock = document()->findBlock(qMax(start, start + length - 1));
    if (!startBlock.isValid() || !endBlock.isValid()) {
        return;
    }

    int remaining = length;
    int currentStart = start;
    for (QTextBlock block = startBlock; block.isValid() && remaining > 0; block = block.next()) {
        const int blockStart = block.position();
        const int localStart = qMax(0, currentStart - blockStart);
        const int available = block.length() - 1 - localStart;
        const int spanLength = qMin(remaining, qMax(0, available));
        if (spanLength > 0 && block.blockNumber() < m_blockRanges.size()) {
            m_blockRanges[block.blockNumber()].append(
                HighlightSpan{localStart, spanLength, kind});
        }
        remaining -= spanLength;
        currentStart = block.position() + block.length() - 1;
        if (block == endBlock) {
            break;
        }
    }
}

void TreeSitterSyntaxHighlighter::applyFormat(SpanKind kind, QTextCharFormat *format) const
{
    if (format == nullptr) {
        return;
    }

    switch (kind) {
    case SpanKind::Keyword:
        format->setForeground(m_darkMode ? QColor("#7cb7ff") : QColor("#0b57d0"));
        format->setFontWeight(QFont::Bold);
        break;
    case SpanKind::Type:
        format->setForeground(m_darkMode ? QColor("#c792ea") : QColor("#6f42c1"));
        break;
    case SpanKind::Namespace:
        format->setForeground(m_darkMode ? QColor("#4fc1ff") : QColor("#0550ae"));
        break;
    case SpanKind::String:
        format->setForeground(m_darkMode ? QColor("#a5d6a7") : QColor("#0a7f2e"));
        break;
    case SpanKind::Number:
        format->setForeground(m_darkMode ? QColor("#9ccc65") : QColor("#116329"));
        break;
    case SpanKind::Comment:
        format->setForeground(m_darkMode ? QColor("#7f8b97") : QColor("#6a737d"));
        break;
    case SpanKind::Function:
        format->setForeground(m_darkMode ? QColor("#82aaff") : QColor("#005cc5"));
        break;
    case SpanKind::Preprocessor:
        format->setForeground(m_darkMode ? QColor("#ffcb6b") : QColor("#9a6700"));
        break;
    case SpanKind::Macro:
        format->setForeground(m_darkMode ? QColor("#ffd166") : QColor("#b26b00"));
        format->setFontWeight(QFont::Bold);
        break;
    case SpanKind::IncludePath:
        format->setForeground(m_darkMode ? QColor("#98d8aa") : QColor("#1a7f37"));
        break;
    case SpanKind::Constant:
        format->setForeground(m_darkMode ? QColor("#f78c6c") : QColor("#b54708"));
        break;
    case SpanKind::Builtin:
        format->setForeground(m_darkMode ? QColor("#ff9cac") : QColor("#b42318"));
        format->setFontItalic(true);
        break;
    case SpanKind::Variable:
        format->setForeground(m_darkMode ? QColor("#e6edf3") : QColor("#24292f"));
        break;
    case SpanKind::Property:
        format->setForeground(m_darkMode ? QColor("#89ddff") : QColor("#0f766e"));
        break;
    case SpanKind::Operator:
        format->setForeground(m_darkMode ? QColor("#f07178") : QColor("#b42318"));
        break;
    case SpanKind::Escape:
        format->setForeground(m_darkMode ? QColor("#ffcb6b") : QColor("#9a6700"));
        format->setFontWeight(QFont::Bold);
        break;
    case SpanKind::Embedded:
        format->setForeground(m_darkMode ? QColor("#c3e88d") : QColor("#3f6212"));
        break;
    }
}

void TreeSitterSyntaxHighlighter::clearTree()
{
    if (m_tree != nullptr) {
        ts_tree_delete(m_tree);
        m_tree = nullptr;
    }
}

void TreeSitterSyntaxHighlighter::clearQuery()
{
    if (m_query != nullptr) {
        ts_query_delete(m_query);
        m_query = nullptr;
    }
}

TreeSitterSyntaxHighlighter::SpanKind TreeSitterSyntaxHighlighter::mapCaptureName(
    const QByteArray &captureName) const
{
    if (captureName.startsWith("keyword")) {
        return SpanKind::Keyword;
    }
    if (captureName.startsWith("type")) {
        return SpanKind::Type;
    }
    if (captureName.startsWith("module")) {
        return SpanKind::Namespace;
    }
    if (captureName.startsWith("string")) {
        return SpanKind::String;
    }
    if (captureName.startsWith("comment")) {
        return SpanKind::Comment;
    }
    if (captureName.startsWith("number")) {
        return SpanKind::Number;
    }
    if (captureName.startsWith("function")) {
        return SpanKind::Function;
    }
    if (captureName.startsWith("constant") || captureName == "constructor") {
        return SpanKind::Constant;
    }
    if (captureName.startsWith("variable.builtin")) {
        return SpanKind::Builtin;
    }
    if (captureName.startsWith("variable")) {
        return SpanKind::Variable;
    }
    if (captureName.startsWith("property")) {
        return SpanKind::Property;
    }
    if (captureName.startsWith("operator")) {
        return SpanKind::Operator;
    }
    if (captureName.startsWith("escape")) {
        return SpanKind::Escape;
    }
    if (captureName.startsWith("embedded") || captureName.startsWith("punctuation.special")) {
        return SpanKind::Embedded;
    }
    return SpanKind::Preprocessor;
}
