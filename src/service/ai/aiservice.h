#pragma once

#include "config/appconfig.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class OpenAiClient;

struct AiToolContext
{
    std::function<QString()> getCurrentProblem;
    std::function<QString()> getCurrentLanguage;
    std::function<QString()> getCurrentCode;
    std::function<QString()> getTestInput;
    std::function<QString()> getTestOutput;
    std::function<void(const QString &)> replaceCode;
    std::function<void(const QString &)> replaceInput;
    std::function<void(const QString &callId, const QString &stdinText)> runTest;
    std::function<void(const QString &callId)> submitCode;
};

class AiService : public QObject
{
    Q_OBJECT

public:
    explicit AiService(OpenAiClient *client, QObject *parent = nullptr);

    void setConfig(const OpenAiConfig &config);
    void setToolContext(const AiToolContext &context);
    OpenAiConfig config() const;
    void ask(const QString &question,
             const QString &problemDetail = QString(),
             const QString &sourceCode = QString(),
             const QString &testInput = QString(),
             const QString &testOutput = QString());
    void translateProblem(const QString &description,
                          const QString &inputSpec,
                          const QString &outputSpec,
                          const QString &hint);
    void completeToolCall(const QString &callId, const QString &outputText);
    void failToolCall(const QString &callId, const QString &message);

signals:
    void thinkingChanged(bool thinking);
    void responseDelta(const QString &text);
    void responseReady(const QString &text);
    void failed(const QString &message);
    void problemTranslationReady(const QString &description,
                                 const QString &inputSpec,
                                 const QString &outputSpec,
                                 const QString &hint);
    void toolCallRequested(const QString &toolName,
                           const QString &callId,
                           const QJsonObject &arguments);

private:
    QString buildUserPrompt(const QString &question,
                            const QString &problemDetail,
                            const QString &sourceCode,
                            const QString &testInput,
                            const QString &testOutput) const;
    QString buildProblemTranslationPrompt(const QString &description,
                                          const QString &inputSpec,
                                          const QString &outputSpec,
                                          const QString &hint) const;
    QString buildToolResultPrompt(const QString &toolName,
                                  const QString &toolOutput,
                                  const QString &problemDetail,
                                  const QString &sourceCode,
                                  const QString &testInput,
                                  const QString &testOutput) const;
    QJsonObject buildRequestPayload(const QString &question,
                                    const QString &problemDetail,
                                    const QString &sourceCode,
                                    const QString &testInput,
                                    const QString &testOutput) const;
    QJsonArray buildTools() const;
    QJsonObject makeMessageItem(const QString &role, const QString &text) const;
    void sendRequest();
    void sendResponseFallback();
    void sendChatCompletionFallback();
    void handleCompletedResponse(const QJsonObject &response);
    void processToolCall(const QJsonObject &callObject);
    QJsonObject makeToolOutputItem(const QString &callId, const QString &outputText) const;
    QString extractResponseText(const QJsonObject &response) const;
    QJsonObject parseArguments(const QString &argumentsText) const;
    QJsonObject parseTranslationObject(const QString &text) const;

private:
    OpenAiClient *m_client = nullptr;
    OpenAiConfig m_config;
    AiToolContext m_toolContext;
    QString m_systemPrompt;
    QString m_question;
    QString m_problemDetail;
    QString m_sourceCode;
    QString m_testInput;
    QString m_testOutput;
    QJsonArray m_inputItems;
    QString m_accumulatedResponse;
    QString m_currentResponseText;
    QString m_debugLog;
    QString m_pendingToolCallId;
    QString m_pendingToolName;
    bool m_thinking = false;
    bool m_translationMode = false;
    bool m_translationChatFallbackTried = false;
    bool m_responseNonStreamFallbackTried = false;
    bool m_chatCompletionFallbackTried = false;
};
