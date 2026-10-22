#pragma once

#include "network/openjudgeclient.h"
#include "parser/problemparser.h"
#include "parser/resultparser.h"
#include "parser/submitparser.h"
#include "repository/meta/problemmetarepository.h"

#include <QHash>
#include <QWidget>

class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QObject;
class QPlainTextEdit;
class QPushButton;
class QTextEdit;
class QListWidget;
class QListWidgetItem;
class QFrame;
class QSplitter;
class QStackedWidget;
class QWidget;

class ProblemPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProblemPage(QWidget *parent = nullptr);
    void setDarkMode(bool dark);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void openProblem(const QString &problemTitle = QString());
    void showProblemLoadedFromFavorites(const ProblemPageInfo &problemPageInfo);
    void showProblem(const ProblemPageInfo &problemPageInfo);
    void showProblemLoadFailed(const QString &message);
    void openSubmit(const ProblemPageInfo &problemPageInfo);
    void showLoadingSubmitOptions(bool loading);
    void showSubmitPageLoaded(const SubmitPageInfo &submitPageInfo,
                              const QString &defaultLanguage);
    void showSubmitting(bool submitting);
    void showTesting(bool testing);
    void showMissingLanguage();
    void showTestResult(const QString &text);
    void showTestFailed(const QString &message);
    void showSubmitPayloadBuilt(const QString &languageValue,
                                const QByteArray &payload);
    void showSubmitResult(const NetworkResult &result);
    void showSubmitFailed(const QString &message);
    void setAiConfigSummary(const QString &summary);
    void showAiThinking(bool thinking);
    void appendAiResponse(const QString &delta);
    void showAiResponse(const QString &text);
    void showAiFailed(const QString &message);
    void showOriginalProblemText();
    void applyCachedProblemTranslation(const QString &description,
                                       const QString &inputSpec,
                                       const QString &outputSpec,
                                       const QString &hint);
    bool hasCachedProblemTranslation() const;
    void showCachedProblemTranslation();
    void showProblemTranslating(bool translating);
    void applyProblemTranslation(const QString &description,
                                 const QString &inputSpec,
                                 const QString &outputSpec,
                                 const QString &hint);
    void showProblemTranslationFailed(const QString &message);
    void setSourceCodeText(const QString &text);
    void setTestInputText(const QString &text);
    void appendResultPageInfo(const ResultPageInfo &resultPageInfo);
    void appendResultFailure(const QString &message);
    QString currentLanguageLabel() const;
    QString currentLanguageValue() const;
    QString currentProblemDetailText() const;
    QString currentSourceCode() const;
    QString currentTestInput() const;
    QString currentTestOutput() const;
    QString currentProblemDescription() const;
    QString currentProblemInputSpec() const;
    QString currentProblemOutputSpec() const;
    QString currentProblemHint() const;
    bool isProblemTranslating() const;
    void setFavoriteEnabled(bool enabled);
    void setSubmitEnabled(bool enabled);
    void setProblemMeta(const ProblemMeta &meta);

signals:
    void backRequested();
    void homeRequested();
    void themeToggleRequested(bool dark);
    void translateProblemRequested();
    void favoriteRequested();
    void saveProblemMetaRequested(const ProblemMeta &meta);
    void aiAskRequested(const QString &question);
    void testRequested(const QString &languageLabel,
                       const QString &sourceText,
                       const QString &stdinText);
    void submitRequested(const QString &language, const QString &sourceText);

private:
    void saveCurrentDraft();
    void restoreDraftOrStarterCode(const ProblemPageInfo &problemPageInfo);
    void setToolsExpanded(bool expanded);
    void setAiPanelVisible(bool visible);
    void setNotesPanelVisible(bool visible);
    void redistributeWorkspacePanels();
    void constrainWindowToScreen();
    void updateSplitterInteractivity();
    void setResultTab(bool showTestTab);
    void resetSubmitPanel();
    void updateCodeHighlightLanguage();
    void appendAiTranscriptBlock(const QString &title, const QString &body);
    void refreshAiResponseView();
    void refreshProblemDetailView();

    QLabel *m_titleLabel = nullptr;
    QFrame *m_toolsFrame = nullptr;
    QPushButton *m_toolsToggleButton = nullptr;
    QWidget *m_toolsPanel = nullptr;
    QWidget *m_collapsedToolsPanel = nullptr;
    QPushButton *m_backToolButton = nullptr;
    QPushButton *m_favoriteToolButton = nullptr;
    QPushButton *m_aiToolButton = nullptr;
    QPushButton *m_notesToolButton = nullptr;
    QPushButton *m_collapsedBackButton = nullptr;
    QPushButton *m_collapsedFavoriteButton = nullptr;
    QPushButton *m_collapsedAiButton = nullptr;
    QPushButton *m_collapsedNotesButton = nullptr;
    QTextEdit *m_detailTextEdit = nullptr;
    QPushButton *m_openInBrowserButton = nullptr;
    QFrame *m_aiFrame = nullptr;
    QLabel *m_aiConfigLabel = nullptr;
    QPlainTextEdit *m_aiPromptEdit = nullptr;
    QPushButton *m_aiAskButton = nullptr;
    QPushButton *m_showOriginalButton = nullptr;
    QPushButton *m_translateButton = nullptr;
    QTextEdit *m_aiResponseTextEdit = nullptr;
    QObject *m_codeHighlighter = nullptr;
    QFrame *m_notesFrame = nullptr;
    QComboBox *m_taskStatusCombo = nullptr;
    QComboBox *m_difficultyCombo = nullptr;
    QLineEdit *m_tagsEdit = nullptr;
    QPlainTextEdit *m_noteEdit = nullptr;
    QCheckBox *m_reviewCheck = nullptr;
    QPushButton *m_saveMetaButton = nullptr;
    QComboBox *m_languageComboBox = nullptr;
    QPlainTextEdit *m_codeEdit = nullptr;
    QPushButton *m_submitButton = nullptr;
    QPushButton *m_inputButton = nullptr;
    QPushButton *m_loadSampleButton = nullptr;
    QPushButton *m_testTabButton = nullptr;
    QPushButton *m_submitTabButton = nullptr;
    QSplitter *m_contentSplitter = nullptr;
    QSplitter *m_workspaceSplitter = nullptr;
    QSplitter *m_submitPaneSplitter = nullptr;
    QStackedWidget *m_resultStack = nullptr;
    QSplitter *m_testPaneSplitter = nullptr;
    QTextEdit *m_testInputTextEdit = nullptr;
    QTextEdit *m_testResultTextEdit = nullptr;
    QTextEdit *m_submitResultTextEdit = nullptr;
    QString m_lastSubmitPreview;
    bool m_toolsExpanded = true;
    bool m_testing = false;
    bool m_aiPanelVisible = false;
    bool m_notesPanelVisible = false;
    bool m_aiThinking = false;
    QString m_aiTranscript;
    QString m_aiResponseBuffer;
    bool m_darkMode = false;
    ProblemPageInfo m_displayedProblemInfo;
    bool m_hasDisplayedProblemInfo = false;
    QString m_translatedDescription;
    QString m_translatedInputSpec;
    QString m_translatedOutputSpec;
    QString m_translatedHint;
    QString m_problemTranslationStatus;
    bool m_hasProblemTranslation = false;
    bool m_problemTranslationLoading = false;
    bool m_showingOriginalProblem = false;
    QString m_currentProblemUrl;
    QHash<QString, QString> m_codeDraftByProblemUrl;
    QHash<QString, QString> m_languageDraftByProblemUrl;
};
