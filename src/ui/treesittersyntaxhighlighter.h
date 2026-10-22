#pragma once

#include <QObject>
#include <QSyntaxHighlighter>

struct TSParser;
struct TSTree;
struct TSQuery;
struct TSQueryCursor;
struct TSLanguage;

class TreeSitterSyntaxHighlighter final : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum class LanguageMode
    {
        PlainText,
        Cpp,
        Python
    };

    explicit TreeSitterSyntaxHighlighter(QTextDocument *parent = nullptr);
    ~TreeSitterSyntaxHighlighter() override;

    void setLanguageMode(LanguageMode mode);
    void setDarkMode(bool dark);

protected:
    void highlightBlock(const QString &text) override;

private:
    enum class SpanKind
    {
        Keyword,
        Type,
        Namespace,
        String,
        Number,
        Comment,
        Function,
        Preprocessor,
        Macro,
        IncludePath,
        Constant,
        Builtin,
        Variable,
        Property,
        Operator,
        Escape,
        Embedded
    };

    struct HighlightSpan
    {
        int start = 0;
        int length = 0;
        SpanKind kind = SpanKind::Keyword;
    };

    struct QuerySpec
    {
        const TSLanguage *language = nullptr;
        QString queryPath;
    };

    QuerySpec buildQuerySpec(LanguageMode mode) const;
    void scheduleParse();
    void reloadQuery();
    void parseDocument();
    void addCppSemanticSpans(const QByteArray &utf8, const QVector<int> &byteToUtf16);
    void collectCppSemanticNode(void *nodePtr,
                                const QByteArray &utf8,
                                const QVector<int> &byteToUtf16);
    void addQueryCaptures(const QVector<int> &byteToUtf16);
    void addSpan(int start, int length, SpanKind kind);
    void applyFormat(SpanKind kind, QTextCharFormat *format) const;
    void clearTree();
    void clearQuery();
    SpanKind mapCaptureName(const QByteArray &captureName) const;

    LanguageMode m_languageMode = LanguageMode::PlainText;
    bool m_darkMode = false;
    bool m_parseScheduled = false;
    bool m_parsing = false;
    TSParser *m_parser = nullptr;
    TSTree *m_tree = nullptr;
    TSQuery *m_query = nullptr;
    TSQueryCursor *m_queryCursor = nullptr;
    QVector<QVector<HighlightSpan>> m_blockRanges;
};
