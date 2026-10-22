#include "ui/pages/problempage.h"
#include "ui/lightmodeiconhelper.h"
#include "ui/treesittersyntaxhighlighter.h"

#include "config/apppaths.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QGuiApplication>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRect>
#include <QRegularExpression>
#include <QKeyEvent>
#include <QShortcut>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
bool g_problemPageDarkMode = false;

QString renderAiTranscriptMarkdown(const QString &transcript)
{
    if (transcript.trimmed().isEmpty()) {
        return QString();
    }

    static const QRegularExpression blockStartRegex(
        QStringLiteral("(?m)^(User|Assistant)\\n"));
    QRegularExpressionMatchIterator iterator = blockStartRegex.globalMatch(transcript);

    QList<QRegularExpressionMatch> matches;
    while (iterator.hasNext()) {
        matches.append(iterator.next());
    }

    if (matches.isEmpty()) {
        return transcript;
    }

    QStringList sections;
    for (int i = 0; i < matches.size(); ++i) {
        const QRegularExpressionMatch &match = matches.at(i);
        const QString title = match.captured(1);
        const int bodyStart = match.capturedEnd(0);
        const int bodyEnd = (i + 1 < matches.size()) ? matches.at(i + 1).capturedStart(0)
                                                     : transcript.size();
        QString body = transcript.mid(bodyStart, bodyEnd - bodyStart);
        while (body.endsWith("\n\n")) {
            body.chop(2);
        }

        if (title == "User") {
            sections << QString("### User\n\n```text\n%1\n```").arg(body);
        } else {
            sections << QString("### Assistant\n\n%1").arg(body);
        }
    }

    return sections.join("\n\n");
}

void writeStartupLog(const QString &message)
{
    QFile file(QDir(AppPaths::dataDir()).filePath("startup.log"));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
           << " | " << message << '\n';
}

class IdeCodeEditor;

class LineNumberArea final : public QWidget
{
public:
    explicit LineNumberArea(IdeCodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    IdeCodeEditor *m_editor = nullptr;
};

class IdeCodeEditor final : public QPlainTextEdit
{
public:
    explicit IdeCodeEditor(QWidget *parent = nullptr) : QPlainTextEdit(parent)
    {
        m_lineNumberArea = new LineNumberArea(this);

        connect(this,
                &QPlainTextEdit::blockCountChanged,
                this,
                [this](int) { updateLineNumberAreaWidth(); });
        connect(this,
                &QPlainTextEdit::updateRequest,
                this,
                [this](const QRect &rect, int dy) {
                    if (dy != 0) {
                        m_lineNumberArea->scroll(0, dy);
                    } else {
                        m_lineNumberArea->update(
                            0, rect.y(), m_lineNumberArea->width(), rect.height());
                    }

                    if (rect.contains(viewport()->rect())) {
                        updateLineNumberAreaWidth();
                    }
                });

        updateLineNumberAreaWidth();
    }

    int lineNumberAreaWidth() const
    {
        int digits = 1;
        int maxValue = qMax(1, blockCount());
        while (maxValue >= 10) {
            maxValue /= 10;
            ++digits;
        }
        return 16 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    }

    void updateLineNumberAreaWidth()
    {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void lineNumberAreaPaintEvent(QPaintEvent *event)
    {
        QPainter painter(m_lineNumberArea);
        painter.fillRect(event->rect(),
                         g_problemPageDarkMode ? QColor("#121920")
                                               : QColor("#f4f6f8"));
        painter.setPen(g_problemPageDarkMode ? QColor("#7f8b97")
                                             : QColor("#8a94a1"));

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                const QString number = QString::number(blockNumber + 1);
                painter.drawText(0,
                                 top,
                                 m_lineNumberArea->width() - 8,
                                 fontMetrics().height(),
                                 Qt::AlignRight,
                                 number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event == nullptr) {
            QPlainTextEdit::keyPressEvent(event);
            return;
        }

        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            && event->modifiers() == Qt::NoModifier) {
            QTextCursor cursor = textCursor();
            const QString blockText = cursor.block().text();
            const int positionInBlock = cursor.position() - cursor.block().position();
            const QString beforeCursor = blockText.left(qMax(0, positionInBlock));
            const QString indent = leadingWhitespace(beforeCursor);
            const QString trimmedBeforeCursor = beforeCursor.trimmed();

            if (trimmedBeforeCursor.endsWith('{')) {
                const QString innerIndent = indent + indentUnit(indent);
                cursor.insertText("\n" + innerIndent + "\n" + indent);
                if (cursor.movePosition(QTextCursor::Up)) {
                    cursor.movePosition(QTextCursor::EndOfLine);
                }
                setTextCursor(cursor);
                return;
            }

            QString nextIndent = indent;
            if (endsWithIndentTrigger(trimmedBeforeCursor)) {
                nextIndent += indentUnit(indent);
            }

            cursor.insertText("\n" + nextIndent);
            return;
        }

        const QString text = event->text();
        if (text.size() == 1 && !text.at(0).isNull()) {
            const QChar input = text.at(0);
            const QString closing = pairedClosingText(input);
            if (!closing.isEmpty()) {
                QTextCursor cursor = textCursor();
                const QString selected = cursor.selectedText();
                if (!selected.isEmpty()) {
                    cursor.insertText(QString(input) + selected + closing);
                    setTextCursor(cursor);
                    return;
                }

                cursor.insertText(QString(input) + closing);
                cursor.movePosition(QTextCursor::Left);
                setTextCursor(cursor);
                return;
            }

            if (isClosingChar(input)) {
                QTextCursor cursor = textCursor();
                if (!cursor.hasSelection()) {
                    if (cursor.position() < document()->characterCount() - 1
                        && cursor.movePosition(QTextCursor::Right,
                                               QTextCursor::KeepAnchor,
                                               1)
                        && cursor.selectedText() == QString(input)) {
                        cursor.clearSelection();
                        setTextCursor(cursor);
                        return;
                    }
                    cursor.clearSelection();
                }
            }
        }

        QPlainTextEdit::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QPlainTextEdit::resizeEvent(event);
        const QRect area = contentsRect();
        m_lineNumberArea->setGeometry(
            QRect(area.left(), area.top(), lineNumberAreaWidth(), area.height()));
    }

private:
    QString leadingWhitespace(const QString &text) const
    {
        QString result;
        for (const QChar ch : text) {
            if (ch == ' ' || ch == '\t') {
                result += ch;
            } else {
                break;
            }
        }
        return result;
    }

    bool endsWithIndentTrigger(const QString &text) const
    {
        return text.endsWith('{') || text.endsWith('(') || text.endsWith('[');
    }

    QString indentUnit(const QString &currentIndent) const
    {
        return currentIndent.contains('\t') && !currentIndent.contains(' ')
            ? QString("\t")
            : QString("    ");
    }

    QString pairedClosingText(QChar input) const
    {
        switch (input.unicode()) {
        case '(':
            return ")";
        case '[':
            return "]";
        case '{':
            return "}";
        case '"':
            return "\"";
        case '\'':
            return "'";
        default:
            return QString();
        }
    }

    bool isClosingChar(QChar input) const
    {
        return input == ')' || input == ']' || input == '}' || input == '"'
               || input == '\'';
    }

    QWidget *m_lineNumberArea = nullptr;
};

LineNumberArea::LineNumberArea(IdeCodeEditor *editor)
    : QWidget(static_cast<QWidget *>(editor)), m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor == nullptr ? 0 : m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    if (m_editor != nullptr) {
        m_editor->lineNumberAreaPaintEvent(event);
    }
}

QString formatProblemDetail(const ProblemPageInfo &problemPageInfo,
                            const QString &translatedDescription = QString(),
                            const QString &translatedInputSpec = QString(),
                            const QString &translatedOutputSpec = QString(),
                            const QString &translatedHint = QString(),
                            const QString &translationStatus = QString())
{
    const QString titleColor = g_problemPageDarkMode ? "#f3f6f9" : "#1f2328";
    const QString metaColor = g_problemPageDarkMode ? "#aebbc8" : "#526056";
    const QString sectionTitleColor = g_problemPageDarkMode ? "#e4ebf2" : "#243029";
    const QString bodyColor = g_problemPageDarkMode ? "#d8e0e8" : "#2f3a33";

    const auto esc = [](const QString &text) {
        QString html = text.toHtmlEscaped();
        html.replace("\r\n", "\n");
        html.replace('\r', '\n');
        html.replace('\n', "<br/>");
        return html;
    };
    const auto section = [&](const QString &title, const QString &body) {
        if (body.trimmed().isEmpty()) {
            return QString();
        }
        return QString(
                   "<div style='margin-top:18px;'>"
                   "<div style='font-size:15px;font-weight:600;color:%1;margin-bottom:8px;'>%2</div>"
                   "<div style='font-size:14px;line-height:1.65;color:%3;'>%4</div>"
                   "</div>")
            .arg(sectionTitleColor, title, bodyColor, body);
    };

    QString html;
    html += "<div style='margin-bottom:14px;white-space:nowrap;'>";
    if (!problemPageInfo.timeLimit.trimmed().isEmpty()) {
        html += QString(
                    "<span style='padding:2px 0px;font-size:12px;color:%1;'>"
                    "Time Limit: %2</span>")
                    .arg(metaColor, problemPageInfo.timeLimit.toHtmlEscaped());
    }
    if (!problemPageInfo.timeLimit.trimmed().isEmpty()
        && !problemPageInfo.memoryLimit.trimmed().isEmpty()) {
        html += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    }
    if (!problemPageInfo.memoryLimit.trimmed().isEmpty()) {
        html += QString(
                    "<span style='padding:2px 0px;font-size:12px;color:%1;'>"
                    "Memory Limit: %2</span>")
                    .arg(metaColor, problemPageInfo.memoryLimit.toHtmlEscaped());
    }
    html += "</div>";

    if (!translationStatus.trimmed().isEmpty()) {
        html += QString(
                    "<div style='margin:8px 0 2px 0;font-size:12px;color:%1;'>%2</div>")
                    .arg(metaColor, translationStatus.toHtmlEscaped());
    }

    html += section("Description",
                    esc(translatedDescription.isEmpty() ? problemPageInfo.description
                                                        : translatedDescription));
    html += section("Input",
                    esc(translatedInputSpec.isEmpty() ? problemPageInfo.inputSpec
                                                      : translatedInputSpec));
    html += section("Output",
                    esc(translatedOutputSpec.isEmpty() ? problemPageInfo.outputSpec
                                                       : translatedOutputSpec));
    html += section("Sample Input", esc(problemPageInfo.sampleInput));
    html += section("Sample Output", esc(problemPageInfo.sampleOutput));
    html += section("Hint",
                    esc(translatedHint.isEmpty() ? problemPageInfo.hint
                                                 : translatedHint));

    return html;
}

QString extractFirstMatch(const QString &text, const QString &pattern)
{
    const QRegularExpression regex(
        pattern,
        QRegularExpression::CaseInsensitiveOption
            | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = regex.match(text);
    return match.hasMatch() ? match.captured(1).simplified() : QString();
}

QString classifySubmitResponse(const NetworkResult &result, const QString &bodyText)
{
    if (!result.ok) {
        return "network error";
    }
    if (result.statusCode == 401) {
        return "authentication required";
    }
    if (bodyText.trimmed().startsWith("<")) {
        if (bodyText.contains("Host requires authentication",
                              Qt::CaseInsensitive)) {
            return "authentication page";
        }
        if (result.finalUrl != result.requestUrl) {
            return "redirected html page";
        }
        return "html page";
    }
    if (bodyText.trimmed().startsWith("{")
        || bodyText.trimmed().startsWith("[")) {
        return "json-like text";
    }
    return "plain text or unknown";
}

QString formatSubmitPayloadPreview(const QString &languageLabel,
                                   const QString &languageValue,
                                   const QString &sourceText,
                                   const QByteArray &payload)
{
    QString preview = QString::fromUtf8(payload);
    if (preview.size() > 1200) {
        preview = preview.left(1200) + "\n...[truncated]";
    }

    return QString(
               "Submit Request Preview\n"
               "Selected label: %1\n"
               "Selected value: %2\n"
               "Source length: %3\n\n"
               "Payload:\n%4")
        .arg(languageLabel.isEmpty() ? "<none>" : languageLabel,
             languageValue.isEmpty() ? "<none>" : languageValue,
             QString::number(sourceText.size()),
             preview);
}

QString formatSubmitResult(const NetworkResult &result)
{
    const QString bodyText = QString::fromUtf8(result.body);
    const QString responseType = classifySubmitResponse(result, bodyText);
    QString statusText;
    if (result.ok && result.statusCode >= 200 && result.statusCode < 400) {
        if (responseType == "result-page") {
            statusText = "Submitted";
        } else if (responseType == "submit-page") {
            statusText = "Submit failed";
        } else {
            statusText = "Submit response received";
        }
    } else {
        statusText = "Submit failed";
    }

    QString errorText;
    if (!result.ok) {
        errorText = result.errorString.trimmed();
    }
    if (errorText.isEmpty() && result.statusCode >= 400) {
        errorText = bodyText.trimmed();
    }
    if (errorText.size() > 800) {
        errorText = errorText.left(800) + "\n...[truncated]";
    }

    QString text = QString("Submit Status: %1").arg(statusText);
    if (!errorText.isEmpty()) {
        text += "\n\nError:\n" + errorText;
    }
    return text;
}

QString formatResultPageInfo(const ResultPageInfo &resultPageInfo)
{
    QString text = QString("Judge Status: %1")
                       .arg(resultPageInfo.statusText.isEmpty()
                                ? "<none>"
                                : resultPageInfo.statusText);
    if (!resultPageInfo.detailTitle.isEmpty()) {
        text += "\n\n" + resultPageInfo.detailTitle;
    }
    if (!resultPageInfo.detailText.isEmpty()) {
        text += "\n" + resultPageInfo.detailText;
    }
    return text;
}
}

QSize ProblemPage::minimumSizeHint() const
{
    return QSize(0, 0);
}

QSize ProblemPage::sizeHint() const
{
    return QWidget::sizeHint().expandedTo(QSize(960, 640));
}
ProblemPage::ProblemPage(QWidget *parent)
    : QWidget(parent)
{
    writeStartupLog("ProblemPage: constructor begin");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *topFrame = new QFrame(this);
    topFrame->setObjectName("problemTopFrame");
    topFrame->setFixedHeight(80);
    auto *topLayout = new QHBoxLayout(topFrame);
    topLayout->setContentsMargins(24, 18, 24, 18);
    topLayout->setSpacing(16);

    m_titleLabel = new QLabel("Problem", topFrame);
    m_titleLabel->setObjectName("problemTitleLabel");
    auto *homeButton = new QPushButton("Home", topFrame);
    homeButton->setObjectName("problemTopActionButton");
    auto *themeButton = new QPushButton("Dark Mode", topFrame);
    themeButton->setObjectName("problemTopActionButton");
    m_translateButton = new QPushButton("Translate", topFrame);
    m_translateButton->setObjectName("problemRefreshButton");

    topLayout->addWidget(m_titleLabel, 1);
    topLayout->addWidget(homeButton, 0, Qt::AlignRight);
    topLayout->addWidget(themeButton, 0, Qt::AlignRight);
    writeStartupLog("ProblemPage: top frame complete");

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(4);

    m_toolsFrame = new QFrame(this);
    m_toolsFrame->setObjectName("problemLeftFrame");
    auto *leftLayout = new QVBoxLayout(m_toolsFrame);
    leftLayout->setContentsMargins(20, 18, 20, 18);
    leftLayout->setSpacing(12);

    m_toolsToggleButton = new QPushButton(m_toolsFrame);
    m_toolsToggleButton->setObjectName("problemToolsToggleButton");

    m_toolsPanel = new QWidget(m_toolsFrame);
    auto *toolsPanelLayout = new QVBoxLayout(m_toolsPanel);
    toolsPanelLayout->setContentsMargins(0, 0, 0, 0);
    toolsPanelLayout->setSpacing(10);

    m_backToolButton = new QPushButton("Back", m_toolsPanel);
    m_backToolButton->setObjectName("problemToolButton");
    m_favoriteToolButton = new QPushButton("Favorite Current Problem", m_toolsPanel);
    m_favoriteToolButton->setObjectName("problemToolButton");
    m_aiToolButton = new QPushButton("AI", m_toolsPanel);
    m_aiToolButton->setObjectName("problemToolButton");
    m_notesToolButton = new QPushButton("Notes", m_toolsPanel);
    m_notesToolButton->setObjectName("problemToolButton");
    toolsPanelLayout->addWidget(m_backToolButton);
    toolsPanelLayout->addWidget(m_favoriteToolButton);
    toolsPanelLayout->addWidget(m_aiToolButton);
    toolsPanelLayout->addWidget(m_notesToolButton);
    toolsPanelLayout->addStretch();

    m_collapsedToolsPanel = new QWidget(m_toolsFrame);
    m_collapsedToolsPanel->setObjectName("problemCollapsedToolsPanel");
    auto *collapsedLayout = new QVBoxLayout(m_collapsedToolsPanel);
    collapsedLayout->setContentsMargins(0, 0, 0, 0);
    collapsedLayout->setSpacing(10);

    m_collapsedBackButton = new QPushButton("B", m_collapsedToolsPanel);
    m_collapsedBackButton->setObjectName("problemToolIconButton");
    m_collapsedBackButton->setToolTip("Back");
    m_collapsedFavoriteButton = new QPushButton("F", m_collapsedToolsPanel);
    m_collapsedFavoriteButton->setObjectName("problemToolIconButton");
    m_collapsedFavoriteButton->setToolTip("Favorite Current Problem");
    m_collapsedAiButton = new QPushButton("Ai", m_collapsedToolsPanel);
    m_collapsedAiButton->setObjectName("problemToolIconButton");
    m_collapsedAiButton->setToolTip("AI");
    m_collapsedNotesButton = new QPushButton("N", m_collapsedToolsPanel);
    m_collapsedNotesButton->setObjectName("problemToolIconButton");
    m_collapsedNotesButton->setToolTip("Notes");
    collapsedLayout->addWidget(m_collapsedBackButton);
    collapsedLayout->addWidget(m_collapsedFavoriteButton);
    collapsedLayout->addWidget(m_collapsedAiButton);
    collapsedLayout->addWidget(m_collapsedNotesButton);
    collapsedLayout->addStretch();

    leftLayout->addWidget(m_toolsToggleButton);
    leftLayout->addWidget(m_toolsPanel);
    leftLayout->addWidget(m_collapsedToolsPanel);
    leftLayout->addStretch();
    writeStartupLog("ProblemPage: tools frame complete");

    auto *problemFrame = new QFrame(this);
    problemFrame->setObjectName("problemMiddleFrame");
    problemFrame->setMinimumWidth(0);
    auto *problemLayout = new QVBoxLayout(problemFrame);
    problemLayout->setContentsMargins(20, 18, 20, 18);
    problemLayout->setSpacing(14);

    auto *problemLabel = new QLabel("Problem", problemFrame);
    problemLabel->setObjectName("problemSectionLabel");

    auto *problemHeaderLayout = new QHBoxLayout();
    problemHeaderLayout->setContentsMargins(0, 0, 0, 0);
    problemHeaderLayout->setSpacing(12);
    problemHeaderLayout->addWidget(problemLabel);
    problemHeaderLayout->addStretch();
    m_openInBrowserButton = new QPushButton("Web", problemFrame);
    m_openInBrowserButton->setObjectName("problemRefreshButton");
    m_openInBrowserButton->setToolTip("Open this problem in the system browser");
    m_showOriginalButton = new QPushButton("Original", problemFrame);
    m_showOriginalButton->setObjectName("problemRefreshButton");
    problemHeaderLayout->addWidget(m_openInBrowserButton, 0, Qt::AlignRight);
    problemHeaderLayout->addWidget(m_showOriginalButton, 0, Qt::AlignRight);
    problemHeaderLayout->addWidget(m_translateButton, 0, Qt::AlignRight);

    m_detailTextEdit = new QTextEdit(problemFrame);
    m_detailTextEdit->setObjectName("problemDetailText");
    m_detailTextEdit->setReadOnly(true);

    problemLayout->addLayout(problemHeaderLayout);
    problemLayout->addWidget(m_detailTextEdit, 1);
    writeStartupLog("ProblemPage: problem frame complete");

    auto *submitFrame = new QFrame(this);
    submitFrame->setObjectName("problemRightFrame");
    submitFrame->setMinimumWidth(0);
    auto *submitLayout = new QVBoxLayout(submitFrame);
    submitLayout->setContentsMargins(20, 18, 20, 18);
    submitLayout->setSpacing(14);

    auto *submitLabel = new QLabel("Submit", submitFrame);
    submitLabel->setObjectName("problemSectionLabel");

    m_languageComboBox = new QComboBox(submitFrame);
    m_languageComboBox->setObjectName("problemLanguageCombo");
    m_languageComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_languageComboBox->setMinimumContentsLength(1);
    m_codeEdit = new IdeCodeEditor(submitFrame);
    m_codeEdit->setObjectName("problemCodeEdit");
    m_codeEdit->setPlaceholderText("Write your source code here.");
    m_codeEdit->setMinimumHeight(180);
    m_codeEdit->setTabStopDistance(
        m_codeEdit->fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
    m_codeHighlighter = new TreeSitterSyntaxHighlighter(m_codeEdit->document());
    writeStartupLog("ProblemPage: code editor complete");
    auto *resultTabLayout = new QHBoxLayout();
    resultTabLayout->setContentsMargins(0, 0, 0, 0);
    resultTabLayout->setSpacing(8);

    m_testTabButton = new QPushButton("test", submitFrame);
    m_testTabButton->setObjectName("problemResultTabButton");
    m_testTabButton->setCheckable(true);
    m_submitTabButton = new QPushButton("submit", submitFrame);
    m_submitTabButton->setObjectName("problemResultTabButton");
    m_submitTabButton->setCheckable(true);
    resultTabLayout->addWidget(m_testTabButton);
    resultTabLayout->addWidget(m_submitTabButton);

    m_loadSampleButton = new QPushButton("sample", submitFrame);
    m_loadSampleButton->setObjectName("problemInputButton");
    m_loadSampleButton->setToolTip("Load sample input into the test input box");
    m_inputButton = new QPushButton("input", submitFrame);
    m_inputButton->setObjectName("problemInputButton");
    m_submitButton = new QPushButton("Submit Code", submitFrame);
    m_submitButton->setObjectName("problemSubmitButton");
    resultTabLayout->addStretch();
    resultTabLayout->addWidget(m_loadSampleButton, 0, Qt::AlignRight);
    resultTabLayout->addWidget(m_inputButton, 0, Qt::AlignRight);
    resultTabLayout->addWidget(m_submitButton, 0, Qt::AlignRight);

    m_resultStack = new QStackedWidget(submitFrame);
    m_resultStack->setObjectName("problemResultStack");
    m_testPaneSplitter = new QSplitter(Qt::Horizontal, m_resultStack);
    m_testPaneSplitter->setObjectName("problemTestPaneSplitter");
    m_testPaneSplitter->setChildrenCollapsible(false);
    m_testPaneSplitter->setHandleWidth(8);

    m_testInputTextEdit = new QTextEdit(m_testPaneSplitter);
    m_testInputTextEdit->setObjectName("problemResultText");
    m_testInputTextEdit->setReadOnly(false);
    m_testInputTextEdit->setMinimumHeight(140);
    m_testInputTextEdit->setPlaceholderText("Write test input here.");

    m_testResultTextEdit = new QTextEdit(m_testPaneSplitter);
    m_testResultTextEdit->setObjectName("problemResultText");
    m_testResultTextEdit->setReadOnly(false);
    m_testResultTextEdit->setMinimumHeight(140);
    m_testResultTextEdit->setReadOnly(true);
    m_testResultTextEdit->setPlaceholderText("Test output will appear here.");
    m_testPaneSplitter->setStretchFactor(0, 1);
    m_testPaneSplitter->setStretchFactor(1, 1);
    m_testPaneSplitter->setSizes({320, 320});

    m_submitResultTextEdit = new QTextEdit(m_resultStack);
    m_submitResultTextEdit->setObjectName("problemResultText");
    m_submitResultTextEdit->setReadOnly(true);
    m_submitResultTextEdit->setMinimumHeight(140);
    m_resultStack->addWidget(m_testPaneSplitter);
    m_resultStack->addWidget(m_submitResultTextEdit);

    auto *resultPanel = new QWidget(submitFrame);
    auto *resultPanelLayout = new QVBoxLayout(resultPanel);
    resultPanelLayout->setContentsMargins(0, 0, 0, 0);
    resultPanelLayout->setSpacing(14);
    resultPanelLayout->addLayout(resultTabLayout);
    resultPanelLayout->addWidget(m_resultStack, 1);

    m_submitPaneSplitter = new QSplitter(Qt::Vertical, submitFrame);
    m_submitPaneSplitter->setObjectName("problemSubmitPaneSplitter");
    m_submitPaneSplitter->setChildrenCollapsible(false);
    m_submitPaneSplitter->setHandleWidth(8);
    m_submitPaneSplitter->addWidget(m_codeEdit);
    m_submitPaneSplitter->addWidget(resultPanel);
    m_submitPaneSplitter->setStretchFactor(0, 3);
    m_submitPaneSplitter->setStretchFactor(1, 2);
    m_submitPaneSplitter->setSizes({360, 240});

    submitLayout->addWidget(submitLabel);
    submitLayout->addWidget(m_languageComboBox);
    submitLayout->addWidget(m_submitPaneSplitter, 1);
    writeStartupLog("ProblemPage: submit frame complete");

    m_aiFrame = new QFrame(this);
    m_aiFrame->setObjectName("problemAiFrame");
    m_aiFrame->setMinimumWidth(0);
    auto *aiLayout = new QVBoxLayout(m_aiFrame);
    aiLayout->setContentsMargins(20, 18, 20, 18);
    aiLayout->setSpacing(14);

    auto *aiLabel = new QLabel("AI", m_aiFrame);
    aiLabel->setObjectName("problemSectionLabel");
    m_aiConfigLabel = new QLabel("Config:", m_aiFrame);
    m_aiConfigLabel->setObjectName("problemAiMetaLabel");
    auto *aiPromptLabel = new QLabel("Prompt", m_aiFrame);
    aiPromptLabel->setObjectName("problemAiFieldLabel");
    m_aiPromptEdit = new QPlainTextEdit(m_aiFrame);
    m_aiPromptEdit->setObjectName("problemCodeEdit");
    m_aiPromptEdit->setPlaceholderText("Ask AI about the current problem, code, or test result.");
    m_aiPromptEdit->setMinimumHeight(120);
    m_aiAskButton = new QPushButton("Ask", m_aiFrame);
    m_aiAskButton->setObjectName("problemSubmitButton");
    auto *aiResponseLabel = new QLabel("Response", m_aiFrame);
    aiResponseLabel->setObjectName("problemAiFieldLabel");
    m_aiResponseTextEdit = new QTextEdit(m_aiFrame);
    m_aiResponseTextEdit->setObjectName("problemResultText");
    m_aiResponseTextEdit->setReadOnly(true);
    m_aiResponseTextEdit->setPlaceholderText("AI response will appear here.");
    auto *aiPresetLayout = new QGridLayout();
    aiPresetLayout->setContentsMargins(0, 0, 0, 0);
    aiPresetLayout->setHorizontalSpacing(8);
    aiPresetLayout->setVerticalSpacing(8);
    struct AiPreset
    {
        QString label;
        QString prompt;
    };
    const QList<AiPreset> aiPresets = {
        {QString::fromUtf8("\xE8\xA7\xA3\xE9\x87\x8A\xE9\xA2\x98\xE6\x84\x8F"), QString::fromUtf8("\xE8\xAF\xB7\xE7\x94\xA8\xE7\xAE\x80\xE6\xB4\x81\xE4\xB8\xAD\xE6\x96\x87\xE8\xA7\xA3\xE9\x87\x8A\xE8\xBF\x99\xE9\x81\x93\xE9\xA2\x98\xE7\x9A\x84\xE9\xA2\x98\xE6\x84\x8F\xE3\x80\x81\xE8\xBE\x93\xE5\x85\xA5\xE8\xBE\x93\xE5\x87\xBA\xE6\xA0\xBC\xE5\xBC\x8F\xE5\x92\x8C\xE6\xA0\xB7\xE4\xBE\x8B\xE5\x90\xAB\xE4\xB9\x89\xE3\x80\x82")},
        {QString::fromUtf8("\xE5\x86\x99\xE6\x9A\xB4\xE5\x8A\x9B\xE8\xA7\xA3"), QString::fromUtf8("\xE8\xAF\xB7\xE5\x9F\xBA\xE4\xBA\x8E\xE5\xBD\x93\xE5\x89\x8D\xE9\xA2\x98\xE7\x9B\xAE\xE7\xBB\x99\xE5\x87\xBA\xE4\xB8\x80\xE4\xB8\xAA\xE6\x9C\xB4\xE7\xB4\xA0\xE7\x9A\x84\xE6\x9A\xB4\xE5\x8A\x9B\xE8\xA7\xA3\xE6\xB3\x95\xEF\xBC\x8C\xE5\xB9\xB6\xE7\xAE\x80\xE8\xA6\x81\xE8\xAF\xB4\xE6\x98\x8E\xE6\x80\x9D\xE8\xB7\xAF\xE4\xB8\x8E\xE5\xA4\x8D\xE6\x9D\x82\xE5\xBA\xA6\xE3\x80\x82")},
        {QString::fromUtf8("\xE4\xBC\x98\xE5\x8C\x96\xE5\xA4\x8D\xE6\x9D\x82\xE5\xBA\xA6"), QString::fromUtf8("\xE8\xAF\xB7\xE5\x88\x86\xE6\x9E\x90\xE5\xBD\x93\xE5\x89\x8D\xE4\xBB\xA3\xE7\xA0\x81\xE7\x9A\x84\xE6\x97\xB6\xE9\x97\xB4\xE5\x92\x8C\xE7\xA9\xBA\xE9\x97\xB4\xE5\xA4\x8D\xE6\x9D\x82\xE5\xBA\xA6\xEF\xBC\x8C\xE5\xB9\xB6\xE7\xBB\x99\xE5\x87\xBA\xE6\x9B\xB4\xE4\xBC\x98\xE7\x9A\x84\xE7\xAE\x97\xE6\xB3\x95\xE6\x80\x9D\xE8\xB7\xAF\xE6\x88\x96\xE5\xAE\x9E\xE7\x8E\xB0\xE3\x80\x82")}};
    for (int i = 0; i < aiPresets.size(); ++i) {
        const AiPreset &preset = aiPresets.at(i);
        auto *presetButton = new QPushButton(preset.label, m_aiFrame);
        presetButton->setObjectName("problemRefreshButton");
        presetButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        presetButton->setToolTip(QString::fromUtf8("\xE5\xB0\x86\xE9\xA2\x84\xE8\xAE\xBE\xE6\x8F\x90\xE7\xA4\xBA\xE5\xA1\xAB\xE5\x85\xA5\xE4\xB8\x8B\xE6\x96\xB9\xE8\xBE\x93\xE5\x85\xA5\xE6\xA1\x86"));
        const QString promptText = preset.prompt;
        connect(presetButton, &QPushButton::clicked, this, [this, promptText]() {
            if (m_aiPromptEdit != nullptr) {
                m_aiPromptEdit->setPlainText(promptText);
                m_aiPromptEdit->setFocus();
            }
        });
        aiPresetLayout->addWidget(presetButton, i / 2, i % 2);
    }

    aiLayout->addWidget(aiLabel);
    aiLayout->addWidget(m_aiConfigLabel);
    aiLayout->addWidget(aiResponseLabel);
    aiLayout->addWidget(m_aiResponseTextEdit, 1);
    aiLayout->addWidget(aiPromptLabel);
    aiLayout->addLayout(aiPresetLayout);
    aiLayout->addWidget(m_aiPromptEdit);
    aiLayout->addWidget(m_aiAskButton, 0, Qt::AlignRight);
    m_aiFrame->hide();
    writeStartupLog("ProblemPage: ai frame complete");

    m_notesFrame = new QFrame(this);
    m_notesFrame->setObjectName("problemAiFrame");
    m_notesFrame->setMinimumWidth(0);
    auto *notesLayout = new QVBoxLayout(m_notesFrame);
    notesLayout->setContentsMargins(20, 18, 20, 18);
    notesLayout->setSpacing(12);

    auto *notesLabel = new QLabel("Notes", m_notesFrame);
    notesLabel->setObjectName("problemSectionLabel");

    auto *statusFieldLabel = new QLabel("Task Status", m_notesFrame);
    statusFieldLabel->setObjectName("problemAiFieldLabel");
    m_taskStatusCombo = new QComboBox(m_notesFrame);
    m_taskStatusCombo->setObjectName("problemLanguageCombo");
    m_taskStatusCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_taskStatusCombo->setMinimumContentsLength(1);
    m_taskStatusCombo->addItem(QString::fromUtf8("\xE6\x9C\xAA\xE5\xBC\x80\xE5\xA7\x8B"), "todo");
    m_taskStatusCombo->addItem(QString::fromUtf8("\xE8\xBF\x9B\xE8\xA1\x8C\xE4\xB8\xAD"), "doing");
    m_taskStatusCombo->addItem(QString::fromUtf8("\xE5\xB7\xB2\xE5\xAE\x8C\xE6\x88\x90"), "done");
    m_taskStatusCombo->addItem(QString::fromUtf8("\xE5\xBE\x85\xE9\x87\x8D\xE5\x81\x9A"), "redo");

    auto *difficultyFieldLabel = new QLabel("Difficulty", m_notesFrame);
    difficultyFieldLabel->setObjectName("problemAiFieldLabel");
    m_difficultyCombo = new QComboBox(m_notesFrame);
    m_difficultyCombo->setObjectName("problemLanguageCombo");
    m_difficultyCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_difficultyCombo->setMinimumContentsLength(1);
    m_difficultyCombo->addItem(QString::fromUtf8("\xE6\x9C\xAA\xE8\xAE\xBE\xE7\xBD\xAE"), 0);
    for (int level = 1; level <= 5; ++level) {
        m_difficultyCombo->addItem(QString::number(level), level);
    }

    auto *tagsFieldLabel = new QLabel("Tags (comma separated)", m_notesFrame);
    tagsFieldLabel->setObjectName("problemAiFieldLabel");
    m_tagsEdit = new QLineEdit(m_notesFrame);
    m_tagsEdit->setObjectName("problemLanguageCombo");
    m_tagsEdit->setPlaceholderText("dp, binary search, ...");

    m_reviewCheck = new QCheckBox(QString::fromUtf8("\xE5\x8A\xA0\xE5\x85\xA5\xE9\x94\x99\xE9\xA2\x98\xE6\x9C\xAC"), m_notesFrame);
    m_reviewCheck->setObjectName("problemAiFieldLabel");

    auto *noteFieldLabel = new QLabel("Note", m_notesFrame);
    noteFieldLabel->setObjectName("problemAiFieldLabel");
    m_noteEdit = new QPlainTextEdit(m_notesFrame);
    m_noteEdit->setObjectName("problemCodeEdit");
    m_noteEdit->setPlaceholderText("Write your personal note for this problem.");

    m_saveMetaButton = new QPushButton("Save", m_notesFrame);
    m_saveMetaButton->setObjectName("problemSubmitButton");

    notesLayout->addWidget(notesLabel);
    notesLayout->addWidget(statusFieldLabel);
    notesLayout->addWidget(m_taskStatusCombo);
    notesLayout->addWidget(difficultyFieldLabel);
    notesLayout->addWidget(m_difficultyCombo);
    notesLayout->addWidget(tagsFieldLabel);
    notesLayout->addWidget(m_tagsEdit);
    notesLayout->addWidget(m_reviewCheck);
    notesLayout->addWidget(noteFieldLabel);
    notesLayout->addWidget(m_noteEdit, 1);
    notesLayout->addWidget(m_saveMetaButton, 0, Qt::AlignRight);
    m_notesFrame->hide();
    writeStartupLog("ProblemPage: notes frame complete");

    m_contentSplitter = new QSplitter(Qt::Horizontal, this);
    m_contentSplitter->setObjectName("problemContentSplitter");
    m_contentSplitter->setChildrenCollapsible(false);
    m_contentSplitter->setHandleWidth(4);
    m_contentSplitter->addWidget(problemFrame);
    m_workspaceSplitter = new QSplitter(Qt::Horizontal, m_contentSplitter);
    m_workspaceSplitter->setObjectName("problemWorkspaceSplitter");
    m_workspaceSplitter->setChildrenCollapsible(false);
    m_workspaceSplitter->setHandleWidth(4);
    m_workspaceSplitter->addWidget(submitFrame);
    m_workspaceSplitter->addWidget(m_aiFrame);
    m_workspaceSplitter->addWidget(m_notesFrame);
    m_workspaceSplitter->setStretchFactor(0, 4);
    m_workspaceSplitter->setStretchFactor(1, 2);
    m_workspaceSplitter->setStretchFactor(2, 2);
    m_workspaceSplitter->setSizes({720, 0, 0});
    m_contentSplitter->addWidget(m_workspaceSplitter);
    m_contentSplitter->setStretchFactor(0, 1);
    m_contentSplitter->setStretchFactor(1, 1);
    m_contentSplitter->setSizes({640, 640});
    connect(m_contentSplitter, &QSplitter::splitterMoved, this, [this](int, int) {
        constrainWindowToScreen();
    updateSplitterInteractivity();
    });
    connect(m_workspaceSplitter, &QSplitter::splitterMoved, this, [this](int, int) {
        constrainWindowToScreen();
    updateSplitterInteractivity();
    });

    bottomLayout->addWidget(m_toolsFrame, 1);
    bottomLayout->addWidget(m_contentSplitter, 4);

    layout->addWidget(topFrame);
    layout->addLayout(bottomLayout, 1);

    homeButton->setToolTip("Home");
    themeButton->setToolTip("Dark Mode");
    m_backToolButton->setToolTip("Back");
    m_favoriteToolButton->setToolTip("Favorite Current Problem");
    m_collapsedBackButton->setToolTip("Back");
    m_collapsedFavoriteButton->setToolTip("Favorite Current Problem");
    LightModeIconHelper::applyIcon(homeButton, "homepage.svg");
    LightModeIconHelper::applyIcon(themeButton, "dark-mode.png");
    LightModeIconHelper::applyIcon(m_collapsedBackButton, "back.svg");
    LightModeIconHelper::applyIcon(m_collapsedFavoriteButton, "favorite.png");
    writeStartupLog("ProblemPage: layout assembly complete");

    setStyleSheet(
        "ProblemPage { background: #f3f1eb; }"
        "#problemTopFrame, #problemLeftFrame, #problemMiddleFrame, #problemAiFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#problemRightFrame {"
        "  background: #fbfaf7;"
        "  border: 1px solid #ded8cc;"
        "  border-radius: 16px;"
        "}"
        "#problemContentSplitter::handle {"
        "  background: transparent;"
        "}"
        "#problemContentSplitter::handle:hover {"
        "  background: rgba(18, 52, 59, 0.08);"
        "}"
        "#problemSubmitPaneSplitter::handle {"
        "  background: transparent;"
        "}"
        "#problemSubmitPaneSplitter::handle:hover {"
        "  background: rgba(18, 52, 59, 0.08);"
        "}"
        "#problemWorkspaceSplitter::handle {"
        "  background: transparent;"
        "}"
        "#problemWorkspaceSplitter::handle:hover {"
        "  background: rgba(18, 52, 59, 0.08);"
        "}"
        "#problemTestPaneSplitter::handle {"
        "  background: transparent;"
        "}"
        "#problemTestPaneSplitter::handle:hover {"
        "  background: rgba(18, 52, 59, 0.08);"
        "}"
        "#problemTitleLabel {"
        "  font-size: 28px;"
        "  font-weight: 600;"
        "  color: #1f2328;"
        "}"
        "#problemSectionLabel {"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#problemAiMetaLabel, #problemAiFieldLabel {"
        "  color: #526056;"
        "}"
        "#problemToolsToggleButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  text-align: left;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  color: #2f3a33;"
        "}"
        "#problemToolsToggleButton:hover {"
        "  color: #12343b;"
        "}"
        "#problemBackButton, #problemSubmitButton, #problemInputButton {"
        "  min-width: 88px;"
        "  padding: 8px 14px;"
        "  border: 1px solid #cdd7cf;"
        "  border-radius: 10px;"
        "  background: #f7f5ef;"
        "  color: #243029;"
        "}"
        "#problemBackButton:hover, #problemSubmitButton:hover, #problemInputButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#problemTopActionButton {"
        "  min-width: 36px;"
        "  padding: 6px;"
        "  border: none;"
        "  border-radius: 0px;"
        "  background: transparent;"
        "  color: #243029;"
        "}"
        "#problemTopActionButton:hover {"
        "  background: transparent;"
        "}"
        "#problemRefreshButton {"
        "  min-width: 88px;"
        "  padding: 8px 14px;"
        "  border: 1px solid #cdd7cf;"
        "  border-radius: 10px;"
        "  background: #f7f5ef;"
        "  color: #243029;"
        "}"
        "#problemRefreshButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#problemResultTabButton {"
        "  min-width: 72px;"
        "  padding: 6px 12px;"
        "  border: 1px solid #d9d4c8;"
        "  border-radius: 10px;"
        "  background: #f7f5ef;"
        "  color: #526056;"
        "}"
        "#problemResultTabButton:checked {"
        "  background: #e4efe7;"
        "  color: #1f2328;"
        "  border-color: #b9cabd;"
        "}"
        "#problemToolButton {"
        "  padding: 10px 12px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  text-align: left;"
        "}"
        "#problemToolButton:hover, #problemToolIconButton:hover {"
        "  background: #eef4ef;"
        "}"
        "#problemToolIconButton {"
        "  min-width: 36px;"
        "  max-width: 36px;"
        "  min-height: 36px;"
        "  max-height: 36px;"
        "  border: none;"
        "  border-radius: 10px;"
        "  background: transparent;"
        "  color: #2f3a33;"
        "  font-weight: 600;"
        "}"
        "#problemDetailText, #problemResultText, #problemCodeEdit, #problemLanguageCombo {"
        "  background: #fcfbf8;"
        "  border: 1px solid #d9d4c8;"
        "  border-radius: 12px;"
        "  color: #1f2328;"
        "}"
        "#problemRightFrame #problemCodeEdit {"
        "  background: #ffffff;"
        "  border: 1px solid #cfd7e3;"
        "  border-radius: 12px;"
        "  color: #1f2328;"
        "  selection-background-color: #cfe8ff;"
        "  selection-color: #1f2328;"
        "  padding: 10px 10px 10px 0px;"
        "  font-family: 'Consolas', 'Courier New', monospace;"
        "  font-size: 14px;"
        "}"
        "#problemRightFrame #problemCodeEdit:focus {"
        "  border: 1px solid #5b9dff;"
        "}"
        "#problemRightFrame #problemLanguageCombo {"
        "  background: #ffffff;"
        "  border: 1px solid #cfd7e3;"
        "  border-radius: 10px;"
        "  color: #1f2328;"
        "  padding: 6px 10px;"
        "  min-height: 34px;"
        "}"
        "#problemRightFrame #problemLanguageCombo::drop-down {"
        "  border: none;"
        "  width: 24px;"
        "}"
        "#problemRightFrame #problemLanguageCombo QAbstractItemView {"
        "  background: #ffffff;"
        "  border: 1px solid #cfd7e3;"
        "  color: #1f2328;"
        "  selection-background-color: #cfe8ff;"
        "}"
    );
    writeStartupLog("ProblemPage: stylesheet applied");

    setToolsExpanded(true);
    writeStartupLog("ProblemPage: tools expanded set");

    connect(homeButton, &QPushButton::clicked, this, &ProblemPage::homeRequested);
    connect(themeButton, &QPushButton::clicked, this, [this]() {
        emit themeToggleRequested(!m_darkMode);
    });
    connect(m_translateButton,
            &QPushButton::clicked,
            this,
            &ProblemPage::translateProblemRequested);
    connect(m_showOriginalButton, &QPushButton::clicked, this, [this]() {
        showOriginalProblemText();
    });
    connect(m_openInBrowserButton, &QPushButton::clicked, this, [this]() {
        if (!m_currentProblemUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(m_currentProblemUrl));
        }
    });
    connect(m_backToolButton, &QPushButton::clicked, this, &ProblemPage::backRequested);
    connect(m_collapsedBackButton, &QPushButton::clicked, this, &ProblemPage::backRequested);
    connect(m_favoriteToolButton, &QPushButton::clicked, this, &ProblemPage::favoriteRequested);
    connect(m_collapsedFavoriteButton, &QPushButton::clicked, this, &ProblemPage::favoriteRequested);
    connect(m_aiToolButton, &QPushButton::clicked, this, [this]() {
        setAiPanelVisible(!m_aiPanelVisible);
    });
    connect(m_collapsedAiButton, &QPushButton::clicked, this, [this]() {
        setAiPanelVisible(!m_aiPanelVisible);
    });
    connect(m_notesToolButton, &QPushButton::clicked, this, [this]() {
        setNotesPanelVisible(!m_notesPanelVisible);
    });
    connect(m_collapsedNotesButton, &QPushButton::clicked, this, [this]() {
        setNotesPanelVisible(!m_notesPanelVisible);
    });
    connect(m_saveMetaButton, &QPushButton::clicked, this, [this]() {
        if (m_currentProblemUrl.isEmpty()) {
            return;
        }
        ProblemMeta meta;
        meta.problemUrl = m_currentProblemUrl;
        meta.title = m_displayedProblemInfo.title;
        meta.taskStatus = m_taskStatusCombo->currentData().toString();
        meta.difficulty = m_difficultyCombo->currentData().toInt();
        meta.priority = 0;
        meta.reviewFlag = m_reviewCheck->isChecked();
        meta.note = m_noteEdit->toPlainText();
        const QStringList rawTags = m_tagsEdit->text().split(',', Qt::SkipEmptyParts);
        for (const QString &rawTag : rawTags) {
            const QString tag = rawTag.trimmed();
            if (!tag.isEmpty()) {
                meta.tags << tag;
            }
        }
        emit saveProblemMetaRequested(meta);
    });
    connect(
        m_languageComboBox,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            if (!m_currentProblemUrl.isEmpty()) {
                m_languageDraftByProblemUrl.insert(m_currentProblemUrl,
                                                   currentLanguageValue());
            }
            updateCodeHighlightLanguage();
        });
    connect(
        m_toolsToggleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setToolsExpanded(!m_toolsExpanded);
        });
    connect(m_aiAskButton, &QPushButton::clicked, this, [this]() {
        const QString question = m_aiPromptEdit->toPlainText().trimmed();
        if (!question.isEmpty()) {
            appendAiTranscriptBlock("User", question);
            refreshAiResponseView();
        }
        emit aiAskRequested(question);
    });
    connect(
        m_loadSampleButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setResultTab(true);
            if (m_hasDisplayedProblemInfo
                && !m_displayedProblemInfo.sampleInput.trimmed().isEmpty()) {
                setTestInputText(m_displayedProblemInfo.sampleInput);
            }
        });
    connect(
        m_inputButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setResultTab(true);
            emit testRequested(currentLanguageLabel(),
                               m_codeEdit->toPlainText(),
                               m_testInputTextEdit->toPlainText());
        });
    connect(
        m_submitButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setResultTab(false);
            emit submitRequested(
                m_languageComboBox->currentData().toString(),
                m_codeEdit->toPlainText());
        });
    connect(
        m_testTabButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setResultTab(true);
        });
    connect(
        m_submitTabButton,
        &QPushButton::clicked,
        this,
        [this]() {
            setResultTab(false);
        });
    writeStartupLog("ProblemPage: signal connections complete");

    m_titleLabel->setText("Problem");
    m_detailTextEdit->setPlainText("Loading problem detail...");
    m_submitResultTextEdit->setPlainText("Preparing submit options...");
    m_testTabButton->setChecked(true);
    m_submitTabButton->setChecked(false);
    m_resultStack->setCurrentWidget(m_testPaneSplitter);
    m_submitButton->setVisible(false);
    m_inputButton->setVisible(true);
    m_loadSampleButton->setVisible(true);
    writeStartupLog("ProblemPage: constructor end");
}

void ProblemPage::setToolsExpanded(bool expanded)
{
    m_toolsExpanded = expanded;
    if (m_toolsFrame != nullptr) {
        m_toolsFrame->setMinimumWidth(expanded ? 0 : 84);
        m_toolsFrame->setMaximumWidth(expanded ? QWIDGETSIZE_MAX : 84);
    }
    if (m_toolsPanel != nullptr) {
        m_toolsPanel->setVisible(expanded);
    }
    if (m_collapsedToolsPanel != nullptr) {
        m_collapsedToolsPanel->setVisible(!expanded);
    }
    const QList<QPushButton *> iconButtons = {m_collapsedBackButton,
                                              m_collapsedFavoriteButton,
                                              m_collapsedAiButton,
                                              m_collapsedNotesButton};
    for (QPushButton *button : iconButtons) {
        if (button != nullptr) {
            button->setVisible(!expanded);
        }
    }
    if (m_toolsToggleButton != nullptr) {
        m_toolsToggleButton->setToolTip(expanded ? "Collapse Tools" : "Expand Tools");
        LightModeIconHelper::applyToolsToggleIcon(m_toolsToggleButton, expanded);
    }
}

void ProblemPage::updateSplitterInteractivity()
{
    const bool allowProblemSubmitResize = !m_aiPanelVisible && !m_notesPanelVisible;

    if (m_contentSplitter != nullptr) {
        for (int i = 1; i < m_contentSplitter->count(); ++i) {
            if (QSplitterHandle *handle = m_contentSplitter->handle(i)) {
                handle->setEnabled(allowProblemSubmitResize);
            }
        }
    }

    if (m_workspaceSplitter != nullptr) {
        for (int i = 1; i < m_workspaceSplitter->count(); ++i) {
            if (QSplitterHandle *handle = m_workspaceSplitter->handle(i)) {
                handle->setEnabled(false);
            }
        }
    }
}
void ProblemPage::constrainWindowToScreen()
{
    QWidget *topLevel = window();
    if (topLevel == nullptr) {
        return;
    }

    QScreen *screen = topLevel->screen();
    if (screen == nullptr) {
        screen = QGuiApplication::screenAt(topLevel->frameGeometry().center());
    }
    if (screen == nullptr) {
        return;
    }

    const QRect available = screen->availableGeometry();
    QSize size = topLevel->size();
    size.setWidth(qMin(size.width(), available.width()));
    size.setHeight(qMin(size.height(), available.height()));
    if (size != topLevel->size()) {
        topLevel->resize(size);
    }

    QRect geometry = topLevel->frameGeometry();
    int x = geometry.x();
    int y = geometry.y();
    if (geometry.right() > available.right()) {
        x -= geometry.right() - available.right();
    }
    if (geometry.bottom() > available.bottom()) {
        y -= geometry.bottom() - available.bottom();
    }
    if (x < available.left()) {
        x = available.left();
    }
    if (y < available.top()) {
        y = available.top();
    }
    if (x != geometry.x() || y != geometry.y()) {
        topLevel->move(x, y);
    }
}
void ProblemPage::redistributeWorkspacePanels()
{
    if (m_contentSplitter == nullptr || m_contentSplitter->count() < 2
        || m_workspaceSplitter == nullptr || m_workspaceSplitter->count() < 3) {
        return;
    }

    QList<int> contentSizes = m_contentSplitter->sizes();
    while (contentSizes.size() < 2) {
        contentSizes << 0;
    }

    const int visibleWorkspaceCount = 1 + (m_aiPanelVisible ? 1 : 0) + (m_notesPanelVisible ? 1 : 0);
    const int totalPanelCount = 1 + visibleWorkspaceCount;
    const int totalWidth = qMax(1,
                                qMax(m_contentSplitter->width(),
                                     qMax(0, contentSizes.at(0))
                                         + qMax(0, contentSizes.at(1))));
    const int paneWidth = totalWidth / qMax(1, totalPanelCount);
    const int remainder = totalWidth - paneWidth * totalPanelCount;
    const int problemWidth = paneWidth;
    const int workspaceWidth = paneWidth * visibleWorkspaceCount + remainder;

    m_contentSplitter->setSizes({problemWidth, workspaceWidth});

    QList<int> sizes;
    sizes << paneWidth;
    sizes << (m_aiPanelVisible ? paneWidth : 0);
    sizes << (m_notesPanelVisible ? paneWidth + remainder : 0);
    if (m_notesPanelVisible) {
        sizes[2] = paneWidth + remainder;
    } else if (m_aiPanelVisible) {
        sizes[1] = paneWidth + remainder;
    } else {
        sizes[0] = paneWidth + remainder;
    }
    m_workspaceSplitter->setSizes(sizes);
    constrainWindowToScreen();
    updateSplitterInteractivity();
}
void ProblemPage::setAiPanelVisible(bool visible)
{
    m_aiPanelVisible = visible;
    if (m_workspaceSplitter == nullptr || m_workspaceSplitter->count() < 2
        || m_aiFrame == nullptr) {
        return;
    }

    if (visible) {
        m_aiFrame->show();
        redistributeWorkspacePanels();
        return;
    }

    redistributeWorkspacePanels();
    m_aiFrame->hide();
}

void ProblemPage::setNotesPanelVisible(bool visible)
{
    m_notesPanelVisible = visible;
    if (m_workspaceSplitter == nullptr || m_workspaceSplitter->count() < 3
        || m_notesFrame == nullptr) {
        return;
    }

    if (visible) {
        m_notesFrame->show();
        redistributeWorkspacePanels();
        return;
    }

    redistributeWorkspacePanels();
    m_notesFrame->hide();
}

void ProblemPage::setResultTab(bool showTestTab)
{
    if (m_resultStack != nullptr) {
        m_resultStack->setCurrentIndex(showTestTab ? 0 : 1);
    }
    if (m_testTabButton != nullptr) {
        m_testTabButton->setChecked(showTestTab);
    }
    if (m_submitTabButton != nullptr) {
        m_submitTabButton->setChecked(!showTestTab);
    }
    if (m_submitButton != nullptr) {
        const bool canSubmit = m_languageComboBox != nullptr
                               && m_languageComboBox->count() > 0
                               && m_languageComboBox->isEnabled();
        m_submitButton->setVisible(!showTestTab);
        m_submitButton->setEnabled(!showTestTab && canSubmit);
    }
    if (m_inputButton != nullptr) {
        m_inputButton->setVisible(showTestTab);
        m_inputButton->setEnabled(showTestTab && !m_testing);
    }
    if (m_loadSampleButton != nullptr) {
        m_loadSampleButton->setVisible(showTestTab);
        m_loadSampleButton->setEnabled(showTestTab && !m_testing);
    }
}

void ProblemPage::openProblem(const QString &problemTitle)
{
    writeStartupLog("ProblemPage::openProblem begin");
    saveCurrentDraft();
    m_currentProblemUrl.clear();
    m_hasDisplayedProblemInfo = false;
    m_displayedProblemInfo = ProblemPageInfo{};
    m_translatedDescription.clear();
    m_translatedInputSpec.clear();
    m_translatedOutputSpec.clear();
    m_translatedHint.clear();
    m_problemTranslationStatus.clear();
    m_hasProblemTranslation = false;
    m_problemTranslationLoading = false;
    m_showingOriginalProblem = false;
    m_titleLabel->setText(problemTitle.isEmpty() ? "Problem" : problemTitle);
    writeStartupLog("ProblemPage::openProblem title set");
    m_detailTextEdit->setPlainText("Loading problem detail...");
    writeStartupLog("ProblemPage::openProblem detail placeholder set");
    resetSubmitPanel();
    writeStartupLog("ProblemPage::openProblem resetSubmitPanel done");
    m_aiTranscript.clear();
    writeStartupLog("ProblemPage::openProblem transcript cleared");
    m_aiResponseBuffer.clear();
    writeStartupLog("ProblemPage::openProblem response buffer cleared");
    refreshAiResponseView();
    writeStartupLog("ProblemPage::openProblem response view refreshed");
    setFavoriteEnabled(false);
    writeStartupLog("ProblemPage::openProblem favorite disabled");
    setSubmitEnabled(false);
    writeStartupLog("ProblemPage::openProblem submit disabled");
    showProblemTranslating(false);
}

void ProblemPage::showProblemLoadedFromFavorites(const ProblemPageInfo &problemPageInfo)
{
    m_currentProblemUrl = problemPageInfo.problemUrl;
    m_displayedProblemInfo = problemPageInfo;
    m_hasDisplayedProblemInfo = true;
    m_translatedDescription.clear();
    m_translatedInputSpec.clear();
    m_translatedOutputSpec.clear();
    m_translatedHint.clear();
    m_problemTranslationStatus.clear();
    m_hasProblemTranslation = false;
    m_problemTranslationLoading = false;
    m_showingOriginalProblem = false;
    m_titleLabel->setText(
        problemPageInfo.title.isEmpty() ? QString("Problem") : problemPageInfo.title);
    refreshProblemDetailView();
    showProblemTranslating(false);
}

void ProblemPage::showProblem(const ProblemPageInfo &problemPageInfo)
{
    m_currentProblemUrl = problemPageInfo.problemUrl;
    m_displayedProblemInfo = problemPageInfo;
    m_hasDisplayedProblemInfo = true;
    m_translatedDescription.clear();
    m_translatedInputSpec.clear();
    m_translatedOutputSpec.clear();
    m_translatedHint.clear();
    m_problemTranslationStatus.clear();
    m_hasProblemTranslation = false;
    m_problemTranslationLoading = false;
    m_showingOriginalProblem = false;
    m_titleLabel->setText(
        problemPageInfo.title.isEmpty() ? QString("Problem") : problemPageInfo.title);
    refreshProblemDetailView();
    showProblemTranslating(false);
}

void ProblemPage::showProblemLoadFailed(const QString &message)
{
    m_hasDisplayedProblemInfo = false;
    m_detailTextEdit->setPlainText(message);
    showProblemTranslating(false);
}

void ProblemPage::openSubmit(const ProblemPageInfo &problemPageInfo)
{
    writeStartupLog("ProblemPage::openSubmit begin");
    resetSubmitPanel();
    writeStartupLog("ProblemPage::openSubmit resetSubmitPanel done");
    restoreDraftOrStarterCode(problemPageInfo);
}

void ProblemPage::showLoadingSubmitOptions(bool loading)
{
    Q_UNUSED(loading);
}

void ProblemPage::showSubmitPageLoaded(const SubmitPageInfo &submitPageInfo,
                                       const QString &defaultLanguage)
{
    writeStartupLog("ProblemPage::showSubmitPageLoaded begin");
    const QSignalBlocker blocker(m_languageComboBox);
    m_languageComboBox->clear();
    for (const SubmitLanguageOption &option : submitPageInfo.languages) {
        m_languageComboBox->addItem(option.label, option.value);
    }
    writeStartupLog("ProblemPage::showSubmitPageLoaded languages filled");

    QString preferredLanguage = defaultLanguage;
    if (!m_currentProblemUrl.isEmpty()) {
        const QString cachedLanguage =
            m_languageDraftByProblemUrl.value(m_currentProblemUrl);
        if (!cachedLanguage.isEmpty()) {
            preferredLanguage = cachedLanguage;
        }
    }

    const int index = m_languageComboBox->findData(preferredLanguage);
    if (index >= 0) {
        m_languageComboBox->setCurrentIndex(index);
    } else {
        const int defaultIndex = m_languageComboBox->findData(defaultLanguage);
        if (defaultIndex >= 0) {
            m_languageComboBox->setCurrentIndex(defaultIndex);
        }
    }
    writeStartupLog("ProblemPage::showSubmitPageLoaded default language selected");

    if (!m_currentProblemUrl.isEmpty() && !currentLanguageValue().isEmpty()) {
        m_languageDraftByProblemUrl.insert(m_currentProblemUrl, currentLanguageValue());
    }

    const bool hasLanguages = m_languageComboBox->count() > 0;
    m_languageComboBox->setEnabled(hasLanguages);
    m_codeEdit->setEnabled(true);
    writeStartupLog("ProblemPage::showSubmitPageLoaded widgets enabled");
    QTimer::singleShot(0, this, [this]() {
        writeStartupLog("ProblemPage::showSubmitPageLoaded deferred highlight begin");
        updateCodeHighlightLanguage();
        writeStartupLog("ProblemPage::showSubmitPageLoaded deferred highlight end");
    });
    m_submitButton->setEnabled(hasLanguages && m_submitTabButton->isChecked());
    m_submitResultTextEdit->setPlainText(
        QString("Submit page loaded.\nAction: %1")
            .arg(submitPageInfo.submitActionUrl));
    writeStartupLog("ProblemPage::showSubmitPageLoaded end");
}

void ProblemPage::showSubmitting(bool submitting)
{
    m_submitButton->setEnabled(
        !submitting && m_submitTabButton->isChecked() && m_languageComboBox->count() > 0
        && m_languageComboBox->isEnabled());
}

void ProblemPage::showTesting(bool testing)
{
    m_testing = testing;
    if (testing) {
        setResultTab(true);
        m_testResultTextEdit->setPlainText("Running test...");
    }
    if (m_inputButton != nullptr) {
        m_inputButton->setEnabled(m_testTabButton->isChecked() && !testing);
    }
    if (m_loadSampleButton != nullptr) {
        m_loadSampleButton->setEnabled(m_testTabButton->isChecked() && !testing);
    }
}

void ProblemPage::showMissingLanguage()
{
    setResultTab(false);
    m_submitResultTextEdit->setPlainText("No language selected.");
}

void ProblemPage::showTestResult(const QString &text)
{
    setResultTab(true);
    m_testResultTextEdit->setPlainText(text);
}

void ProblemPage::showTestFailed(const QString &message)
{
    setResultTab(true);
    m_testResultTextEdit->setPlainText(message);
}

void ProblemPage::showSubmitPayloadBuilt(const QString &languageValue,
                                         const QByteArray &payload)
{
    m_lastSubmitPreview = formatSubmitPayloadPreview(
        currentLanguageLabel(),
        languageValue,
        m_codeEdit->toPlainText(),
        payload);
    setResultTab(false);
    m_submitResultTextEdit->setPlainText("Submitting...");
}

void ProblemPage::showSubmitResult(const NetworkResult &result)
{
    const QString resultText = formatSubmitResult(result);
    setResultTab(false);
    m_submitResultTextEdit->setPlainText(resultText);
}

void ProblemPage::showSubmitFailed(const QString &message)
{
    setResultTab(false);
    m_submitResultTextEdit->setPlainText(message);
}

void ProblemPage::setAiConfigSummary(const QString &summary)
{
    if (m_aiConfigLabel != nullptr) {
        m_aiConfigLabel->setText(summary);
    }
}

void ProblemPage::showAiThinking(bool thinking)
{
    m_aiThinking = thinking;
    if (m_aiAskButton != nullptr) {
        m_aiAskButton->setEnabled(!thinking);
        m_aiAskButton->setText(thinking ? "Asking..." : "Ask AI");
    }
    if (thinking) {
        m_aiResponseBuffer.clear();
        const QString waitingBlock = "Assistant\nWaiting for OpenAI response...";
        if (!m_aiTranscript.endsWith(waitingBlock)) {
            appendAiTranscriptBlock("Assistant", "Waiting for OpenAI response...");
            refreshAiResponseView();
        }
    }
}

void ProblemPage::appendAiResponse(const QString &delta)
{
    m_aiResponseBuffer += delta;
    const QString waitingBlock = "Assistant\nWaiting for OpenAI response...";
    if (m_aiTranscript.endsWith(waitingBlock)) {
        m_aiTranscript.chop(waitingBlock.size());
        m_aiTranscript += "Assistant\n" + m_aiResponseBuffer;
    } else {
        const QString assistantPrefix = "Assistant\n";
        const int assistantIndex = m_aiTranscript.lastIndexOf(assistantPrefix);
        if (assistantIndex >= 0) {
            m_aiTranscript = m_aiTranscript.left(assistantIndex) + assistantPrefix
                             + m_aiResponseBuffer;
        } else {
            appendAiTranscriptBlock("Assistant", m_aiResponseBuffer);
        }
    }
    refreshAiResponseView();
}

void ProblemPage::showAiResponse(const QString &text)
{
    m_aiResponseBuffer = text;
    const QString assistantPrefix = "Assistant\n";
    const int assistantIndex = m_aiTranscript.lastIndexOf(assistantPrefix);
    if (assistantIndex >= 0) {
        m_aiTranscript = m_aiTranscript.left(assistantIndex) + assistantPrefix + text;
    } else {
        appendAiTranscriptBlock("Assistant", text);
    }
    refreshAiResponseView();
}

void ProblemPage::showAiFailed(const QString &message)
{
    const QString assistantPrefix = "Assistant\n";
    const int assistantIndex = m_aiTranscript.lastIndexOf(assistantPrefix);
    if (assistantIndex >= 0) {
        m_aiTranscript = m_aiTranscript.left(assistantIndex) + assistantPrefix + message;
    } else {
        appendAiTranscriptBlock("Assistant", message);
    }
    refreshAiResponseView();
}

void ProblemPage::showOriginalProblemText()
{
    m_showingOriginalProblem = true;
    m_problemTranslationStatus = "Showing original text.";
    refreshProblemDetailView();
    showProblemTranslating(false);
}

void ProblemPage::applyCachedProblemTranslation(const QString &description,
                                                const QString &inputSpec,
                                                const QString &outputSpec,
                                                const QString &hint)
{
    m_translatedDescription = description;
    m_translatedInputSpec = inputSpec;
    m_translatedOutputSpec = outputSpec;
    m_translatedHint = hint;
    m_hasProblemTranslation = !description.trimmed().isEmpty()
                              || !inputSpec.trimmed().isEmpty()
                              || !outputSpec.trimmed().isEmpty()
                              || !hint.trimmed().isEmpty();
    m_showingOriginalProblem = false;
    m_problemTranslationStatus =
        m_hasProblemTranslation ? "Showing cached translation." : QString();
    refreshProblemDetailView();
    showProblemTranslating(false);
}

bool ProblemPage::hasCachedProblemTranslation() const
{
    return m_hasProblemTranslation;
}

void ProblemPage::showCachedProblemTranslation()
{
    if (!m_hasProblemTranslation) {
        return;
    }
    m_showingOriginalProblem = false;
    m_problemTranslationStatus = "Showing cached translation.";
    refreshProblemDetailView();
    showProblemTranslating(false);
}

void ProblemPage::showProblemTranslating(bool translating)
{
    m_problemTranslationLoading = translating;
    if (translating) {
        m_problemTranslationStatus = "Translating description, input, output, and hint...";
    }
    if (m_translateButton == nullptr) {
        refreshProblemDetailView();
        return;
    }

    const bool canTranslate =
        m_hasDisplayedProblemInfo
        && (!m_displayedProblemInfo.description.trimmed().isEmpty()
            || !m_displayedProblemInfo.hint.trimmed().isEmpty());
    if (m_showOriginalButton != nullptr) {
        m_showOriginalButton->setEnabled(m_hasProblemTranslation && !translating);
    }
    m_translateButton->setEnabled(canTranslate && !translating);
    if (translating) {
        m_translateButton->setText("Translating...");
    } else if (m_hasProblemTranslation) {
        m_translateButton->setText("Translated");
    } else {
        m_translateButton->setText("Translate");
    }
    refreshProblemDetailView();
}

void ProblemPage::applyProblemTranslation(const QString &description,
                                          const QString &inputSpec,
                                          const QString &outputSpec,
                                          const QString &hint)
{
    m_translatedDescription = description;
    m_translatedInputSpec = inputSpec;
    m_translatedOutputSpec = outputSpec;
    m_translatedHint = hint;
    m_problemTranslationStatus = "Translation applied.";
    m_hasProblemTranslation = !description.trimmed().isEmpty()
                              || !inputSpec.trimmed().isEmpty()
                              || !outputSpec.trimmed().isEmpty()
                              || !hint.trimmed().isEmpty();
    m_problemTranslationLoading = false;
    m_showingOriginalProblem = false;
    showProblemTranslating(false);
    refreshProblemDetailView();
}

void ProblemPage::showProblemTranslationFailed(const QString &message)
{
    m_problemTranslationLoading = false;
    m_problemTranslationStatus = "Translation failed: " + message;
    showProblemTranslating(false);
    refreshProblemDetailView();
}

void ProblemPage::setSourceCodeText(const QString &text)
{
    if (m_codeEdit != nullptr) {
        m_codeEdit->setPlainText(text);
    }
    if (!m_currentProblemUrl.isEmpty()) {
        m_codeDraftByProblemUrl.insert(m_currentProblemUrl, text);
    }
}

void ProblemPage::setTestInputText(const QString &text)
{
    if (m_testInputTextEdit != nullptr) {
        m_testInputTextEdit->setPlainText(text);
    }
}

void ProblemPage::setProblemMeta(const ProblemMeta &meta)
{
    if (m_taskStatusCombo != nullptr) {
        const int index = m_taskStatusCombo->findData(meta.taskStatus);
        m_taskStatusCombo->setCurrentIndex(index >= 0 ? index : 0);
    }
    if (m_difficultyCombo != nullptr) {
        const int index = m_difficultyCombo->findData(meta.difficulty);
        m_difficultyCombo->setCurrentIndex(index >= 0 ? index : 0);
    }
    if (m_tagsEdit != nullptr) {
        m_tagsEdit->setText(meta.tags.join(", "));
    }
    if (m_reviewCheck != nullptr) {
        m_reviewCheck->setChecked(meta.reviewFlag);
    }
    if (m_noteEdit != nullptr) {
        m_noteEdit->setPlainText(meta.note);
    }
}

void ProblemPage::appendResultPageInfo(const ResultPageInfo &resultPageInfo)
{
    setResultTab(false);
    m_submitResultTextEdit->append("\n\n" + formatResultPageInfo(resultPageInfo));
}

void ProblemPage::appendResultFailure(const QString &message)
{
    setResultTab(false);
    m_submitResultTextEdit->append("\n\nResult page load failed:\n" + message);
}

QString ProblemPage::currentLanguageLabel() const
{
    return m_languageComboBox->currentText();
}

QString ProblemPage::currentLanguageValue() const
{
    return m_languageComboBox->currentData().toString();
}

QString ProblemPage::currentProblemDetailText() const
{
    return m_detailTextEdit == nullptr ? QString() : m_detailTextEdit->toPlainText();
}

QString ProblemPage::currentSourceCode() const
{
    return m_codeEdit == nullptr ? QString() : m_codeEdit->toPlainText();
}

QString ProblemPage::currentTestInput() const
{
    return m_testInputTextEdit == nullptr ? QString() : m_testInputTextEdit->toPlainText();
}

QString ProblemPage::currentTestOutput() const
{
    return m_testResultTextEdit == nullptr ? QString() : m_testResultTextEdit->toPlainText();
}

QString ProblemPage::currentProblemDescription() const
{
    return m_displayedProblemInfo.description;
}

QString ProblemPage::currentProblemInputSpec() const
{
    return m_displayedProblemInfo.inputSpec;
}

QString ProblemPage::currentProblemOutputSpec() const
{
    return m_displayedProblemInfo.outputSpec;
}

QString ProblemPage::currentProblemHint() const
{
    return m_displayedProblemInfo.hint;
}

bool ProblemPage::isProblemTranslating() const
{
    return m_problemTranslationLoading;
}

void ProblemPage::setFavoriteEnabled(bool enabled)
{
    if (m_favoriteToolButton != nullptr) {
        m_favoriteToolButton->setEnabled(enabled);
    }
    if (m_collapsedFavoriteButton != nullptr) {
        m_collapsedFavoriteButton->setEnabled(enabled);
    }
}

void ProblemPage::setSubmitEnabled(bool enabled)
{
    m_languageComboBox->setEnabled(enabled && m_languageComboBox->count() > 0);
    m_codeEdit->setEnabled(enabled && m_languageComboBox->count() > 0);
    m_submitButton->setEnabled(
        enabled && m_submitTabButton->isChecked() && m_languageComboBox->count() > 0);
    if (m_inputButton != nullptr) {
        m_inputButton->setEnabled(
            enabled && m_testTabButton->isChecked() && !m_testing
            && m_languageComboBox->count() > 0);
    }
}

void ProblemPage::resetSubmitPanel()
{
    writeStartupLog("ProblemPage::resetSubmitPanel begin");
    m_testing = false;
    writeStartupLog("ProblemPage::resetSubmitPanel testing false");
    const QSignalBlocker blocker(m_languageComboBox);
    m_languageComboBox->clear();
    writeStartupLog("ProblemPage::resetSubmitPanel language cleared");
    m_languageComboBox->setEnabled(false);
    writeStartupLog("ProblemPage::resetSubmitPanel language disabled");
    m_codeEdit->setEnabled(false);
    writeStartupLog("ProblemPage::resetSubmitPanel code disabled");
    m_submitButton->setEnabled(false);
    writeStartupLog("ProblemPage::resetSubmitPanel submit disabled");
    m_inputButton->setEnabled(false);
    writeStartupLog("ProblemPage::resetSubmitPanel input disabled");
    m_testInputTextEdit->clear();
    writeStartupLog("ProblemPage::resetSubmitPanel test input cleared");
    m_testResultTextEdit->clear();
    writeStartupLog("ProblemPage::resetSubmitPanel test output cleared");
    m_submitResultTextEdit->setPlainText("Preparing submit options...");
    writeStartupLog("ProblemPage::resetSubmitPanel submit result set");
    setResultTab(true);
    writeStartupLog("ProblemPage::resetSubmitPanel result tab set");
    m_lastSubmitPreview.clear();
    writeStartupLog("ProblemPage::resetSubmitPanel end");
}

void ProblemPage::saveCurrentDraft()
{
    if (m_currentProblemUrl.isEmpty() || m_codeEdit == nullptr) {
        return;
    }
    m_codeDraftByProblemUrl.insert(m_currentProblemUrl, m_codeEdit->toPlainText());
}

void ProblemPage::restoreDraftOrStarterCode(const ProblemPageInfo &problemPageInfo)
{
    const QString key = problemPageInfo.problemUrl;
    if (!key.isEmpty() && m_codeDraftByProblemUrl.contains(key)) {
        setSourceCodeText(m_codeDraftByProblemUrl.value(key));
        writeStartupLog("ProblemPage::openSubmit draft restored");
        return;
    }

    if (!problemPageInfo.starterCode.trimmed().isEmpty()) {
        setSourceCodeText(problemPageInfo.starterCode);
        writeStartupLog("ProblemPage::openSubmit starter code applied");
        return;
    }

    setSourceCodeText(QString());
    writeStartupLog("ProblemPage::openSubmit starter code skipped");
}

void ProblemPage::updateCodeHighlightLanguage()
{
    auto *highlighter = dynamic_cast<TreeSitterSyntaxHighlighter *>(m_codeHighlighter);
    if (highlighter == nullptr || m_languageComboBox == nullptr) {
        return;
    }

    const QString label = currentLanguageLabel().toLower();
    const QString value = currentLanguageValue().toLower();

    TreeSitterSyntaxHighlighter::LanguageMode mode =
        TreeSitterSyntaxHighlighter::LanguageMode::PlainText;
    if (label.contains("python") || value.contains("python")) {
        mode = TreeSitterSyntaxHighlighter::LanguageMode::Python;
    } else if (label.contains("g++") || label.contains("gcc")
               || value.contains("g++") || value.contains("gcc")
               || value.contains("cpp") || value.contains("c++")) {
        mode = TreeSitterSyntaxHighlighter::LanguageMode::Cpp;
    }

    highlighter->setLanguageMode(mode);
}

void ProblemPage::appendAiTranscriptBlock(const QString &title, const QString &body)
{
    if (!m_aiTranscript.isEmpty()) {
        m_aiTranscript += "\n\n";
    }
    m_aiTranscript += title + "\n" + body;
}

void ProblemPage::refreshAiResponseView()
{
    if (m_aiResponseTextEdit == nullptr) {
        return;
    }

    QScrollBar *verticalBar = m_aiResponseTextEdit->verticalScrollBar();
    QScrollBar *horizontalBar = m_aiResponseTextEdit->horizontalScrollBar();
    const int verticalValue = verticalBar != nullptr ? verticalBar->value() : 0;
    const int horizontalValue = horizontalBar != nullptr ? horizontalBar->value() : 0;

    if (m_aiTranscript.isEmpty()) {
        m_aiResponseTextEdit->clear();
    } else {
        m_aiResponseTextEdit->setMarkdown(
            renderAiTranscriptMarkdown(m_aiTranscript));
    }

    if (verticalBar != nullptr) {
        verticalBar->setValue(verticalValue);
    }
    if (horizontalBar != nullptr) {
        horizontalBar->setValue(horizontalValue);
    }
}

void ProblemPage::refreshProblemDetailView()
{
    if (!m_hasDisplayedProblemInfo || m_detailTextEdit == nullptr) {
        return;
    }

    m_detailTextEdit->setHtml(formatProblemDetail(
        m_displayedProblemInfo,
        m_hasProblemTranslation && !m_showingOriginalProblem ? m_translatedDescription
                                                             : QString(),
        m_hasProblemTranslation && !m_showingOriginalProblem ? m_translatedInputSpec
                                                             : QString(),
        m_hasProblemTranslation && !m_showingOriginalProblem ? m_translatedOutputSpec
                                                             : QString(),
        m_hasProblemTranslation && !m_showingOriginalProblem ? m_translatedHint
                                                             : QString(),
        m_problemTranslationStatus));
}

void ProblemPage::setDarkMode(bool dark)
{
    m_darkMode = dark;
    g_problemPageDarkMode = dark;
    QString lightStyle = property("_lightStyleSheet").toString();
    if (lightStyle.isEmpty()) {
        lightStyle = styleSheet();
        setProperty("_lightStyleSheet", lightStyle);
    }

    const QString darkOverride =
        "ProblemPage { background: #000000; }"
        "#problemTopFrame, #problemLeftFrame, #problemMiddleFrame, #problemRightFrame, #problemAiFrame {"
        "  background: #1b232c;"
        "  border: 1px solid #2c3844;"
        "}"
        "#problemTitleLabel, #problemSectionLabel, #problemToolsToggleButton, #problemToolButton, #problemToolIconButton, #problemAiConfigLabel, #problemAiFieldLabel, #problemAiMetaLabel {"
        "  color: #d9e1e8;"
        "}"
        "#problemTopActionButton {"
        "  border: none;"
        "  background: transparent;"
        "  color: #e8edf2;"
        "}"
        "#problemRefreshButton, #problemInputButton, #problemSubmitButton, #problemResultTabButton {"
        "  border: 1px solid #3a4652;"
        "  background: #202a34;"
        "  color: #e8edf2;"
        "}"
        "#problemTopActionButton:hover, #problemRefreshButton:hover, #problemInputButton:hover, #problemSubmitButton:hover, #problemResultTabButton:hover, #problemToolButton:hover, #problemToolIconButton:hover {"
        "  background: #26313c;"
        "}"
        "#problemResultTabButton:checked {"
        "  background: #234257;"
        "  border: 1px solid #4d82b8;"
        "  color: #eff8ff;"
        "}"
        "#problemResultTabButton:checked:hover {"
        "  background: #2a4d65;"
        "}"
        "#problemDetailText {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}"
        "#problemRightFrame #problemLanguageCombo {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "}"
        "#problemRightFrame #problemLanguageCombo QAbstractItemView {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}"
        "#problemRightFrame #problemCodeEdit {"
        "  background: #0f141a;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}"
        "#problemRightFrame #problemCodeEdit:focus {"
        "  border: 1px solid #4d82b8;"
        "}"
        "#problemRightFrame #problemResultText, #problemAiFrame #problemResultText {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}"
        "#problemAiFrame #problemCodeEdit {"
        "  background: #121920;"
        "  border: 1px solid #3a4652;"
        "  color: #e8edf2;"
        "  selection-background-color: #295a85;"
        "}"
        "#problemSubmitPaneSplitter::handle, #problemWorkspaceSplitter::handle, #problemContentSplitter::handle {"
        "  background: transparent;"
        "}"
        "#problemSubmitPaneSplitter::handle:hover, #problemWorkspaceSplitter::handle:hover, #problemContentSplitter::handle:hover {"
        "  background: rgba(255, 255, 255, 0.10);"
        "}";

    setStyleSheet(dark ? lightStyle + darkOverride : lightStyle);
    if (auto *highlighter = dynamic_cast<TreeSitterSyntaxHighlighter *>(m_codeHighlighter)) {
        highlighter->setDarkMode(dark);
    }
    if (m_codeEdit != nullptr) {
        m_codeEdit->viewport()->update();
        m_codeEdit->update();
    }
    refreshProblemDetailView();
    showProblemTranslating(m_problemTranslationLoading);
}
